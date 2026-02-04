#!/usr/bin/env python
########################################################
#
#  Stack Trace Decoder (Stateful)
#
########################################################
import sys
import os
import re
import shlex
import subprocess
import hashlib
import shutil
from enum import Enum, auto

class Colors:
    RESET = "\033[0m"
    BOLD = "\033[1m"
    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    BLUE = "\033[34m"
    MAGENTA = "\033[35m"
    CYAN = "\033[36m"
    WHITE = "\033[37m"
    GRAY = "\033[90m"
    LIGHT_BLUE = "\033[94m" # Labels
    
def getAddrColor(val):
    """Return ANSI color code based on memory region."""
    # Stack Canary / Poison values
    if val in [0xDEADBEEF, 0xA5A5A5A5, 0xFEFEFEFE, 0xABABABAB]:
        return Colors.RED

    # Based on typical ESP32 memory map
    # IRAM/IROM (Code)
    if 0x40000000 <= val < 0x50000000:
        return Colors.GREEN
    # DRAM/DROM (Data)
    elif 0x30000000 <= val < 0x40000000:
        return Colors.YELLOW
    # External SPI RAM (if present)
    elif 0x50000000 <= val < 0x60000000:
        return Colors.CYAN
    else:
        return Colors.RESET

def getElfSha256(filepath):
    """Calculate SHA256 of the ELF file to match against dump info."""
    try:
        sha256_hash = hashlib.sha256()
        with open(filepath, "rb") as f:
            for byte_block in iter(lambda: f.read(4096), b""):
                sha256_hash.update(byte_block)
        return sha256_hash.hexdigest()
    except Exception as e:
        return None

def loadElfSymbols(elf_path, nm_tool):
    """
    Load symbols using nm tool on the ELF file.
    Output format of 'nm -n': <address> <type> <name>
    """
    if not nm_tool or not shutil.which(nm_tool):
        return []

    symbols = []
    print(f"Loading symbols from ELF using {nm_tool}...", file=sys.stderr)
    cmd = [nm_tool, '-n', elf_path]
    try:
        # Run nm (capture_output requires python 3.7+)
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False)
        if result.returncode != 0:
             print(f"Warning: nm returned {result.returncode}", file=sys.stderr)
             return []

        for line in result.stdout.splitlines():
            parts = line.split()
            if len(parts) >= 3:
                # Format: Address Type Name
                # 3ffb0000 D _UserExceptionVector
                try:
                    addr = int(parts[0], 16)
                    # type_char = parts[1] 
                    name = parts[2]
                    symbols.append((addr, name))
                except ValueError:
                    pass
        
        print(f"Loaded {len(symbols)} symbols from ELF.", file=sys.stderr)
        return symbols

    except Exception as e:
        print(f"Error running nm: {e}", file=sys.stderr)
        return []

def loadMapSymbols(elf_path):
    """
    Attempt to load symbols from a .map file.
    Returns a sorted list of (address, symbol_name) tuples.
    """
    map_path = os.path.splitext(elf_path)[0] + ".map"
    if not os.path.exists(map_path):
        return []

    symbols = []
    # Regex for standard GNU LD map file entries: "0x000000003ffb0000                _my_variable"
    map_regex = re.compile(r"^\s+(0x[0-9a-fA-F]{8,})\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*$")
    
    try:
        with open(map_path, 'r') as f:
            for line in f:
                if "0x" not in line: continue 
                m = map_regex.match(line)
                if m:
                    addr_val = int(m.group(1), 16)
                    name = m.group(2)
                    symbols.append((addr_val, name))
        
        symbols.sort(key=lambda x: x[0])
        print(f"Loaded {len(symbols)} symbols from map file.", file=sys.stderr)
        return symbols
    except Exception as e:
        print(f"Warning: Failed to parse map file: {e}", file=sys.stderr)
        return []

def findSymbol(address, symbols):
    """Find the closest preceding symbol for a given address."""
    if not symbols: return None
    
    # Simple linear search (can be optimized with bisect)
    best_sym = None
    offset = 0
    
    for sym_addr, sym_name in symbols:
        if sym_addr > address:
            break
        best_sym = (sym_addr, sym_name)
    
    if best_sym:
        offset = address - best_sym[0]
        if offset < 8192: 
            return f"{best_sym[1]}+{offset}" if offset > 0 else best_sym[1]
    return None

class DecoderState(Enum):
    IDLE = auto()
    IN_REGISTERS = auto()
    IN_STACK = auto()

class CrashDecoder:
    def __init__(self, elf_file, tool_path, map_symbols, soc_name='esp32', objdump_tool=None):
        self.elf_file = elf_file
        self.tool_path = tool_path
        self.map_symbols = map_symbols
        self.soc_name = soc_name
        self.objdump_tool = objdump_tool
        self.state = DecoderState.IDLE
        self.addr2line_proc = None
        self.register_buffer = [] # format: (Name, AddrStr)
        self.stack_buffer = [] # format: int_value
        self.stack_lines = [] # Raw text lines for reprint
        
        # Start addr2line process
        # -a: input address, -i: unwind inlines, -f: functions, -C: demangle
        # Note: -p is NOT used as it changes output format to single-line which breaks parsing
        cmd = [self.tool_path, '-aifC', '-e', self.elf_file]
        try:
            self.addr2line_proc = subprocess.Popen(
                cmd, 
                stdin=subprocess.PIPE, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.DEVNULL,
                text=True,
                bufsize=1 # Line buffered
            )
        except Exception as e:
            print(f"Error starting {self.tool_path}: {e}", file=sys.stderr)
            sys.exit(1)

    def close(self):
        if self.addr2line_proc:
            self.addr2line_proc.terminate()

    def resolveCode(self, addr_str):
        """
        Interacts with addr2line to resolve an address.
        Uses 0x00000000 sentinel to sync.
        Returns formatted string.
        """
        if not self.addr2line_proc: return None

        try:
            # Send address and sentinel
            self.addr2line_proc.stdin.write(f"{addr_str}\n0x00000000\n")
            self.addr2line_proc.stdin.flush()
            
            output_lines = []
            found_echo = False
            
            while True:
                line = self.addr2line_proc.stdout.readline()
                if not line: break
                line = line.strip()

                if line == "0x00000000":
                    # Sentinel reached. Consume standard broken lines for 0x0
                    # Standard output for 0x0 without valid mapping is usually:
                    # ??
                    # ??:0
                    self.addr2line_proc.stdout.readline()
                    self.addr2line_proc.stdout.readline()
                    break
                
                # Check for echo of valid address (handle case variance)
                if line.lower() == addr_str.lower():
                    found_echo = True
                    continue

                if found_echo:
                    output_lines.append(line)

            # Format result: Function at File:Line
            # addr2line output (after echo) is usually: FunctionName \n File:Line
            if len(output_lines) >= 2:
                # Handle potentially multiple frames (inlines)
                results = []
                seen_funcs = set()
                for i in range(0, len(output_lines), 2):
                    if i+1 < len(output_lines):
                        func = output_lines[i].strip()
                        loc = output_lines[i+1].strip()
                        
                        # Clean up discriminator info (noisy)
                        loc = re.sub(r'\s*\(discriminator \d+\)', '', loc)

                        if func != "??" and loc != "??:0":
                            if func in seen_funcs:
                                continue # Skip recursive/inlined frames of same function
                            
                            seen_funcs.add(func)
                            results.append(f"{func} at {loc}")
                
                return " | ".join(results) if results else None
                
            return None

        except Exception as e:
            # print(f"Addr2line Error: {e}", file=sys.stderr)
            return None

    def disassemble(self, addr, count=6):
        if not self.objdump_tool: return
        
        # We want to see 'addr' and a few instructions after.
        # Start exactly at addr. 
        # Instructions are 2 or 4 bytes. 
        # 6 * 4 = 24 bytes is safe for "3-4 instructions" even if mixed 16/32bit.
        start = addr
        stop = addr + 24 
        
        cmd = [self.objdump_tool, '-d', 
               '--start-address', f'0x{start:x}', 
               '--stop-address', f'0x{stop:x}', 
               self.elf_file]
               
        try:
             res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
             if res.returncode == 0:
                  print(f"\n  {Colors.BOLD}Disassembly around {Colors.GREEN}0x{start:x}{Colors.RESET}:")
                  lines = res.stdout.splitlines()
                  for line in lines:
                      # Typical objdump line: "400d3f28:	00413603          	l32u	a2,4(sp)"
                      if re.match(r'\s*[0-9a-fA-F]+:', line):
                           print(f"    {Colors.GRAY}{line}{Colors.RESET}")
        except Exception as e:
             pass

    def getExceptionDesc(self, cause_code):
        # RISC-V Exception Codes
        riscv_exceptions = {
            0: "Instruction address misaligned",
            1: "Instruction access fault",
            2: "Illegal instruction",
            3: "Breakpoint",
            4: "Load address misaligned",
            5: "Load access fault",
            6: "Store/AMO address misaligned",
            7: "Store/AMO access fault",
            8: "Environment call from U-mode",
            9: "Environment call from S-mode",
            11: "Environment call from M-mode",
            12: "Instruction page fault",
            13: "Load page fault",
            15: "Store/AMO page fault"
        }
        # Xtensa Exception Codes (Partial list)
        xtensa_exceptions = {
            0: "Illegal Instruction",
            1: "Syscall",
            2: "Instruction Fetch Error",
            3: "Load/Store Error",
            4: "Level 1 Interrupt",
            5: "Alloca",
            6: "Integer Divide by Zero",
            28: "Load Prohibited",
            29: "Store Prohibited"
        }
        
        is_riscv = 'c3' in self.soc_name or 'c2' in self.soc_name or 'c6' in self.soc_name or 'h2' in self.soc_name or 'riscv' in self.tool_path
        
        if is_riscv:
            return riscv_exceptions.get(cause_code, f"Unknown (RISC-V code {cause_code})")
        else:
            return xtensa_exceptions.get(cause_code, f"Unknown (Xtensa code {cause_code})")

    def displaySourceContext(self, file_path, line_num):
        if not file_path or line_num <= 0: return
        
        # Check if file exists
        if not os.path.exists(file_path):
             # Try relative to CWD if path is relative
             if not os.path.exists(os.path.abspath(file_path)):
                 return

        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
                total_lines = len(lines)
                
                start = max(1, line_num - 2)
                end = min(total_lines, line_num + 2)
                
                print(f"\n  {Colors.BOLD}Context in {Colors.MAGENTA}{os.path.basename(file_path)}{Colors.RESET}:")
                for i in range(start, end + 1):
                    prefix = " >" if i == line_num else "  "
                    color = Colors.GREEN if i == line_num else Colors.GRAY
                    # 1-based index for display, 0-based for list
                    code_line = lines[i-1].rstrip()
                    print(f"   {prefix} {i:4d}: {color}{code_line}{Colors.RESET}")
        except Exception:
            pass

    def flushRegisters(self):
        if not self.register_buffer:
            return

        print(f"\n{Colors.BOLD}Register decode{Colors.RESET}")
        
        target_disasm_addr = None
        source_context_candidate = None # (file, line)

        for name, addr_str in self.register_buffer:
            try:
                val = int(addr_str, 16)
                
                # Exception Cause Decoding
                if name in ["MCAUSE", "EXCCAUSE"]:
                    cause_val = val
                    is_interrupt = False
                    
                    # Determine architecture for decoding logic
                    is_riscv = 'c3' in self.soc_name or 'c2' in self.soc_name or 'c6' in self.soc_name or 'h2' in self.soc_name or 'riscv' in self.tool_path

                    if name == "MCAUSE" or is_riscv:
                        # RISC-V: Bit 31 (XLEN-1) indicates interrupt
                        if cause_val & 0x80000000:
                             is_interrupt = True
                             cause_val &= 0x7FFFFFFF
                    
                    # Xtensa EXCCAUSE is 6-bit, no modification needed usually.
                    
                    if not is_interrupt or cause_val < 64: # Sanity check
                        desc = self.getExceptionDesc(cause_val)
                        desc_str = f"{Colors.RED}{desc}{Colors.RESET}"
                        if is_interrupt: desc_str += " (Interrupt)"
                    else:
                        desc_str = "" # Too large/invalid
                        
                    print(f"  {name:8}: {getAddrColor(val)}{addr_str}{Colors.RESET}  ({desc_str})")
                    continue

                # Capture MEPC/PC/EPC for disassembly
                if name in ["MEPC", "PC", "EPC"]:
                    target_disasm_addr = val
                
                res = None
                
                # Check Code (0x4...)
                if 0x40000000 <= val < 0x50000000:
                    res = self.resolveCode(addr_str)
                    # Fallback
                    if not res or "??:0" in res:
                        sym = findSymbol(val, self.map_symbols)
                        if sym:
                             res = sym
                    
                    # If this is the PC, try to capture source context
                    if name in ["MEPC", "PC", "EPC"] and res and " at " in res:
                        # Extract File:Line
                        # Format: func at file:line
                        m = re.search(r' at (.*):(\d+)', res)
                        if m:
                            source_context_candidate = (m.group(1), int(m.group(2)))

                # Check Data (0x3... or 0x5...)
                elif (0x30000000 <= val < 0x40000000) or (0x50000000 <= val < 0x60000000):
                    sym = findSymbol(val, self.map_symbols)
                    if sym:
                        res = sym
                
                # Colorize the address
                addr_colored = f"{getAddrColor(val)}{addr_str}{Colors.RESET}"

                if res and res != "?? at ??:0":
                     # Highlight file:line in magenta if present
                     if " at " in res:
                         res = res.replace(" at ", f" at {Colors.MAGENTA}").replace(" | ", f"{Colors.RESET} | ") + Colors.RESET
                     else:
                         # Symbol only -> Light Blue
                         res = f"{Colors.LIGHT_BLUE}{res}{Colors.RESET}"

                     print(f"  {name:8}: {addr_colored}  ({res})")
                else:
                     print(f"  {name:8}: {addr_colored}")

            except ValueError:
                pass
        
        if source_context_candidate:
             self.displaySourceContext(source_context_candidate[0], source_context_candidate[1])

        if target_disasm_addr:
            self.disassemble(target_disasm_addr)

        print("") # clean separation
        self.register_buffer = []

    def flushStack(self):
        if not self.stack_buffer:
            return

        print(f"{Colors.BOLD}Stack Dump{Colors.RESET}")
        
        # 1. Reprint original lines with coloring
        def color_replacer(match):
            val_str = match.group(0)
            try:
                val = int(val_str, 16)
                return f"{getAddrColor(val)}{val_str}{Colors.RESET}"
            except:
                return val_str

        for line in self.stack_lines:
             print(re.sub(r"0x[0-9a-fA-F]+", color_replacer, line), end='')
        
        # 2. Analyze stack for summary (silent resolution)
        funcs_found = []
        labels_found = []

        for val in self.stack_buffer:
            addr_str = f"0x{val:08x}"
            resolved = False
            addr_colored = f"{getAddrColor(val)}{addr_str}{Colors.RESET}"
            
            # Code (0x4...)
            if 0x40000000 <= val < 0x50000000:
                res = self.resolveCode(addr_str)
                if res and "??:0" not in res:
                    # Colorize Source Location
                    res_display = res.replace(" at ", f" at {Colors.MAGENTA}") + Colors.RESET
                    funcs_found.append(f"{addr_colored} {res_display}")
                    resolved = True
                else:
                    # Fallback to symbol
                    sym = findSymbol(val, self.map_symbols)
                    if sym:
                        funcs_found.append(f"{addr_colored} {Colors.LIGHT_BLUE}{sym}{Colors.RESET}")
                        resolved = True
            
            # If not code, check data (0x3... or 0x5...)
            if not resolved and ((0x30000000 <= val < 0x40000000) or (0x50000000 <= val < 0x60000000)):
                sym = findSymbol(val, self.map_symbols)
                if sym:
                    labels_found.append(f"    {addr_colored} -> {Colors.LIGHT_BLUE}{sym}{Colors.RESET}")
                    resolved = True
        
        # Summary Sections
        if funcs_found:
             print(f"\n{Colors.BOLD}Calculated Stack Trace (Functions):{Colors.RESET}")
             for f in funcs_found:
                 print(f.strip())
        
        if labels_found:
             print(f"\n{Colors.BOLD}Stack Symbols (Data/Labels):{Colors.RESET}")
             for l in labels_found:
                 print(l.strip())

        self.stack_buffer = []
        self.stack_lines = []

    def processLine(self, line):
        raw_line = line
        line = line.strip()

        # State transition detection
        is_reg_start = "Core" in line and "register dump:" in line
        is_stack_start = "Stack memory:" in line
        is_backtrace = "Backtrace:" in line
        
        # Determine End of Blocks
        if self.state == DecoderState.IN_REGISTERS:
            # Registers end if we see Stack memory, Backtrace, or empty line
            if is_stack_start or is_backtrace or not line:
                self.flushRegisters()
                self.state = DecoderState.IDLE
                # Don't return, allow fall-through to handle start of new state

        if self.state == DecoderState.IN_STACK:
            # Check for transitions inside stack block (e.g. register dump appended to line)
            if is_reg_start or (not raw_line.strip() and self.state == DecoderState.IN_STACK):
                 # Current line 'raw_line' might contain stack tokens AND the start trigger
                 # We must ensure we captured the tokens from this line FIRST.
                 
                 # 1. Capture text for reprint
                 self.stack_lines.append(raw_line)
                 
                 # 2. Capture hex values from THIS line if present
                 parts = line.split(':')
                 content = parts[-1] if ':' in line else line
                 hex_vals = re.findall(r"(0x[0-9a-fA-F]{8})", content)
                 if hex_vals:
                    for h in hex_vals:
                        try:
                            # Avoid capturing register values? 
                            # Registers key-vals: MEPC : 0x... 
                            # Stack: 0x... 0x...
                            # If line has "Register dump:", it probably doesn't have stack values AFTER the text?
                            # Input: "... 0x00 0x17 Core 0 register dump:"
                            # Regex will catch 0x00 and 0x17. Good.
                            self.stack_buffer.append(int(h, 16))
                        except: pass

                 # 3. Flush now that we have everything
                 self.flushStack()
                 
                 if is_reg_start:
                     self.state = DecoderState.IN_REGISTERS
                     # We consumed the line in stack_lines (flush_stack printed it). 
                     # Return to avoid double printing or reprocessing as IDLE
                     return 
                 else:
                     self.state = DecoderState.IDLE
                     # If empty line caused end, we consumed it.
                     return

        # State Handling
        if self.state == DecoderState.IDLE:
            print(raw_line, end='')
            if is_reg_start:
                self.state = DecoderState.IN_REGISTERS
            elif is_stack_start:
                self.state = DecoderState.IN_STACK

        elif self.state == DecoderState.IN_REGISTERS:
            # If we just entered IN_REGISTERS from IDLE above, we already printed the header.
            # But if we are continuing:
            if not is_reg_start: # Don't reprint header if we handled it in IDLE
                 print(raw_line, end='')
            
            # Parse registers: "Name : 0xVal"
            # PC      : 0x400d1f28  PS      : 0x00060830
            matches = list(re.finditer(r"([A-Z0-9_]+)\s*:\s*(0x[0-9a-fA-F]+)", line))
            if matches:
                for m in matches:
                    self.register_buffer.append((m.group(1), m.group(2)))

        elif self.state == DecoderState.IN_STACK:
            self.stack_lines.append(raw_line)
            # Collect hex values
            # 3ffb1d90: 0x00000000 0x00000000 ...
            
            parts = line.split(':')
            content = parts[-1] if ':' in line else line
            
            hex_vals = re.findall(r"(0x[0-9a-fA-F]{8})", content)
            if hex_vals:
                for h in hex_vals:
                    try:
                        self.stack_buffer.append(int(h, 16))
                    except: pass

def printLegend():
    print(f"{Colors.BOLD}Color Legend:{Colors.RESET}", file=sys.stderr)
    print(f"  {Colors.GREEN}0x40xxxxxx{Colors.RESET} - Code (IRAM/IROM)      {Colors.MAGENTA}File:Line{Colors.RESET} - Source Location", file=sys.stderr)
    print(f"  {Colors.YELLOW}0x30xxxxxx{Colors.RESET} - Data (DRAM/DROM)      {Colors.LIGHT_BLUE}Symbol{Colors.RESET}    - Function/Variable", file=sys.stderr)
    print(f"  {Colors.CYAN}0x50xxxxxx{Colors.RESET} - External SPI RAM      {Colors.RED}Canary{Colors.RESET}    - Stack Poison/Canary", file=sys.stderr)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: \n\t{sys.argv[0]} <file.elf> [<error-stack.log>]")
        sys.exit(1)

    elf_file = sys.argv[1]

    # Tool Selection
    soc = os.environ.get('SMING_SOC', 'esp32').lower()
    if soc in ['esp32s2', 'esp32s3']:
         tool_name = f"xtensa-{soc}-elf-addr2line"
    elif soc in ['esp32c3', 'esp32c2', 'esp32c6']:
         tool_name = "riscv32-esp-elf-addr2line"
    else:
         tool_name = "xtensa-esp32-elf-addr2line"
    
    if shutil.which(tool_name) is None:
        print(f"Error: '{tool_name}' not found in PATH.", file=sys.stderr)
        sys.exit(1)

    # Helper: Find nm tool
    nm_tool_name = tool_name.replace("addr2line", "gcc-nm")
    if shutil.which(nm_tool_name) is None:
         nm_tool_name = tool_name.replace("addr2line", "nm")
         if shutil.which(nm_tool_name) is None:
             nm_tool_name = None

    # SHA256 Check
    local_hash = getElfSha256(elf_file)
    if local_hash:
        print(f"Local ELF SHA256: {local_hash[:16]}...", file=sys.stderr)

    # Load symbols: Prefer ELF (nm), fallback to Map
    symbols = loadElfSymbols(elf_file, nm_tool_name)
    if not symbols:
         symbols = loadMapSymbols(elf_file)

    printLegend()
    
    # Helper: Find objdump tool
    objdump_tool_name = tool_name.replace("addr2line", "objdump")
    if shutil.which(objdump_tool_name) is None:
         objdump_tool_name = None

    decoder = CrashDecoder(elf_file, tool_name, symbols, soc, objdump_tool_name)

    # Input Stream Setup
    input_stream = sys.stdin
    if len(sys.argv) > 2:
        try:
            input_stream = open(sys.argv[2], 'r')
        except Exception as e:
            print(f"Error opening log file: {e}")
            sys.exit(1)

    try:
        for line in input_stream:
            # SHA256 extraction from stream (stateless check)
            if local_hash and "ELF file SHA256:" in line:
                m = re.search(r"SHA256:\s*([0-9a-fA-F]+)", line)
                if m:
                    remote_hash = m.group(1)
                    if not local_hash.startswith(remote_hash.lower()):
                        print(f"\n{Colors.RED}{Colors.BOLD}WARNING: ELF Checksum Mismatch!{Colors.RESET}")
                        print(f"{Colors.RED}Dump:  {remote_hash}{Colors.RESET}")
                        print(f"{Colors.RED}Local: {local_hash}{Colors.RESET}\n")

            decoder.processLine(line)
            
        # Flush remaining buffers at EOF
        if decoder.state == DecoderState.IN_REGISTERS:
            decoder.flushRegisters()
        elif decoder.state == DecoderState.IN_STACK:
            decoder.flushStack()
            
    except KeyboardInterrupt:
        pass
    finally:
        decoder.close()
        if input_stream is not sys.stdin:
            input_stream.close()

if __name__ == "__main__":
    main()
