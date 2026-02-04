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
import io
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
        sha256Hash = hashlib.sha256()
        with open(filepath, "rb") as f:
            for byteBlock in iter(lambda: f.read(4096), b""):
                sha256Hash.update(byteBlock)
        return sha256Hash.hexdigest()
    except Exception as e:
        return None

def loadElfSymbols(elfPath, nmTool):
    """
    Load symbols using nm tool on the ELF file.
    Output format of 'nm -n': <address> <type> <name>
    """
    if not nmTool or not shutil.which(nmTool):
        return []

    symbols = []
    print(f"Loading symbols from ELF using {nmTool}...", file=sys.stderr)
    cmd = [nmTool, '-n', elfPath]
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

def loadMapSymbols(elfPath):
    """
    Attempt to load symbols from a .map file.
    Returns a sorted list of (address, symbol_name) tuples.
    """
    mapPath = os.path.splitext(elfPath)[0] + ".map"
    if not os.path.exists(mapPath):
        return []

    symbols = []
    # Regex for standard GNU LD map file entries: "0x000000003ffb0000                _my_variable"
    mapRegex = re.compile(r"^\s+(0x[0-9a-fA-F]{8,})\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*$")
    
    try:
        with open(mapPath, 'r') as f:
            for line in f:
                if "0x" not in line: continue 
                m = mapRegex.match(line)
                if m:
                    addrVal = int(m.group(1), 16)
                    name = m.group(2)
                    symbols.append((addrVal, name))
        
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
    bestSym = None
    offset = 0
    
    for symAddr, symName in symbols:
        if symAddr > address:
            break
        bestSym = (symAddr, symName)
    
    if bestSym:
        offset = address - bestSym[0]
        if offset < 8192: 
            return f"{bestSym[1]}+{offset}" if offset > 0 else bestSym[1]
    return None

class DecoderState(Enum):
    IDLE = auto()
    IN_REGISTERS = auto()
    IN_STACK = auto()

class CrashDecoder:
    def __init__(self, elfFile, toolPath, mapSymbols, socName='esp32', objdumpTool=None, interactive=False):
        self.elfFile = elfFile
        self.toolPath = toolPath
        self.mapSymbols = mapSymbols
        self.socName = socName
        self.objdumpTool = objdumpTool
        self.interactive = interactive
        self.state = DecoderState.IDLE
        self.addr2lineProc = None
        self.registerBuffer = [] # format: (Name, AddrStr)
        self.stackBuffer = [] # format: int_value
        self.stackLines = [] # Raw text lines for reprint
        
        # Start addr2line process
        # -a: input address, -i: unwind inlines, -f: functions, -C: demangle
        # Note: -p is NOT used as it changes output format to single-line which breaks parsing
        cmd = [self.toolPath, '-aifC', '-e', self.elfFile]
        try:
            self.addr2lineProc = subprocess.Popen(
                cmd, 
                stdin=subprocess.PIPE, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.DEVNULL,
                text=True,
                bufsize=1 # Line buffered
            )
        except Exception as e:
            print(f"Error starting {self.toolPath}: {e}", file=sys.stderr)
            sys.exit(1)

    def close(self):
        if self.addr2lineProc:
            self.addr2lineProc.terminate()

    def resolveCode(self, addrStr):
        """
        Interacts with addr2line to resolve an address.
        Uses 0x00000000 sentinel to sync.
        Returns formatted string.
        """
        if not self.addr2lineProc: return None

        try:
            # Send address and sentinel
            self.addr2lineProc.stdin.write(f"{addrStr}\n0x00000000\n")
            self.addr2lineProc.stdin.flush()
            
            outputLines = []
            foundEcho = False
            
            while True:
                line = self.addr2lineProc.stdout.readline()
                if not line: break
                line = line.strip()

                if line == "0x00000000":
                    # Sentinel reached. Consume standard broken lines for 0x0
                    # Standard output for 0x0 without valid mapping is usually:
                    # ??
                    # ??:0
                    self.addr2lineProc.stdout.readline()
                    self.addr2lineProc.stdout.readline()
                    break
                
                # Check for echo of valid address (handle case variance)
                if line.lower() == addrStr.lower():
                    foundEcho = True
                    continue

                if foundEcho:
                    outputLines.append(line)

            # Format result: Function at File:Line
            # addr2line output (after echo) is usually: FunctionName \n File:Line
            if len(outputLines) >= 2:
                # Handle potentially multiple frames (inlines)
                results = []
                seenFuncs = set()
                for i in range(0, len(outputLines), 2):
                    if i+1 < len(outputLines):
                        func = outputLines[i].strip()
                        loc = outputLines[i+1].strip()
                        
                        # Clean up discriminator info (noisy)
                        loc = re.sub(r'\s*\(discriminator \d+\)', '', loc)

                        if func != "??" and loc != "??:0":
                            if func in seenFuncs:
                                continue # Skip recursive/inlined frames of same function
                            
                            seenFuncs.add(func)
                            results.append(f"{func} at {loc}")
                
                return " | ".join(results) if results else None
                
            return None

        except Exception as e:
            # print(f"Addr2line Error: {e}", file=sys.stderr)
            return None

    def disassemble(self, addr, count=6):
        if not self.objdumpTool: return
        
        # We want to see 'addr' and a few instructions after.
        # Start exactly at addr. 
        # Instructions are 2 or 4 bytes. 
        # 6 * 4 = 24 bytes is safe for "3-4 instructions" even if mixed 16/32bit.
        start = addr
        stop = addr + 24 
        
        cmd = [self.objdumpTool, '-d', 
               '--start-address', f'0x{start:x}', 
               '--stop-address', f'0x{stop:x}', 
               self.elfFile]
               
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

    def getExceptionDesc(self, causeCode):
        # RISC-V Exception Codes
        riscvExceptions = {
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
        xtensaExceptions = {
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
        
        isRiscv = 'c3' in self.socName or 'c2' in self.socName or 'c6' in self.socName or 'h2' in self.socName or 'riscv' in self.toolPath
        
        if isRiscv:
            return riscvExceptions.get(causeCode, f"Unknown (RISC-V code {causeCode})")
        else:
            return xtensaExceptions.get(causeCode, f"Unknown (Xtensa code {causeCode})")

    def displaySourceContext(self, filePath, lineNum):
        if not filePath or lineNum <= 0: return
        
        # Check if file exists
        if not os.path.exists(filePath):
             # Try relative to CWD if path is relative
             if not os.path.exists(os.path.abspath(filePath)):
                 return

        try:
            with open(filePath, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
                totalLines = len(lines)
                
                start = max(1, lineNum - 2)
                end = min(totalLines, lineNum + 2)
                
                print(f"\n  {Colors.BOLD}Context in {Colors.MAGENTA}{os.path.basename(filePath)}{Colors.RESET}:")
                for i in range(start, end + 1):
                    prefix = " >" if i == lineNum else "  "
                    color = Colors.GREEN if i == lineNum else Colors.GRAY
                    # 1-based index for display, 0-based for list
                    codeLine = lines[i-1].rstrip()
                    print(f"   {prefix} {i:4d}: {color}{codeLine}{Colors.RESET}")
        except Exception:
            pass

    def flushRegisters(self):
        if not self.registerBuffer:
            return

        print(f"\n{Colors.BOLD}Register decode{Colors.RESET}")
        
        targetDisasmAddr = None
        sourceContextCandidate = None # (file, line)

        for name, addrStr in self.registerBuffer:
            try:
                val = int(addrStr, 16)
                
                # Exception Cause Decoding
                if name in ["MCAUSE", "EXCCAUSE"]:
                    causeVal = val
                    isInterrupt = False
                    
                    # Determine architecture for decoding logic
                    isRiscv = 'c3' in self.socName or 'c2' in self.socName or 'c6' in self.socName or 'h2' in self.socName or 'riscv' in self.toolPath

                    if name == "MCAUSE" or isRiscv:
                        # RISC-V: Bit 31 (XLEN-1) indicates interrupt
                        if causeVal & 0x80000000:
                             isInterrupt = True
                             causeVal &= 0x7FFFFFFF
                    
                    # Xtensa EXCCAUSE is 6-bit, no modification needed usually.
                    
                    if not isInterrupt or causeVal < 64: # Sanity check
                        desc = self.getExceptionDesc(causeVal)
                        descStr = f"{Colors.RED}{desc}{Colors.RESET}"
                        if isInterrupt: descStr += " (Interrupt)"
                    else:
                        descStr = "" # Too large/invalid
                        
                    print(f"  {name:8}: {getAddrColor(val)}{addrStr}{Colors.RESET}  ({descStr})")
                    continue

                # Capture MEPC/PC/EPC for disassembly
                if name in ["MEPC", "PC", "EPC"]:
                    targetDisasmAddr = val
                
                res = None
                
                # Check Code (0x4...)
                if 0x40000000 <= val < 0x50000000:
                    res = self.resolveCode(addrStr)
                    # Fallback
                    if not res or "??:0" in res:
                        sym = findSymbol(val, self.mapSymbols)
                        if sym:
                             res = sym
                    
                    # If this is the PC, try to capture source context
                    if name in ["MEPC", "PC", "EPC"] and res and " at " in res:
                        # Extract File:Line
                        # Format: func at file:line
                        m = re.search(r' at (.*):(\d+)', res)
                        if m:
                            sourceContextCandidate = (m.group(1), int(m.group(2)))

                # Check Data (0x3... or 0x5...)
                elif (0x30000000 <= val < 0x40000000) or (0x50000000 <= val < 0x60000000):
                    sym = findSymbol(val, self.mapSymbols)
                    if sym:
                        res = sym
                
                # Colorize the address
                addrColored = f"{getAddrColor(val)}{addrStr}{Colors.RESET}"

                if res and res != "?? at ??:0":
                     # Highlight file:line in magenta if present
                     if " at " in res:
                         res = res.replace(" at ", f" at {Colors.MAGENTA}").replace(" | ", f"{Colors.RESET} | ") + Colors.RESET
                     else:
                         # Symbol only -> Light Blue
                         res = f"{Colors.LIGHT_BLUE}{res}{Colors.RESET}"

                     print(f"  {name:8}: {addrColored}  ({res})")
                else:
                     print(f"  {name:8}: {addrColored}")

            except ValueError:
                pass
        
        if sourceContextCandidate:
             self.displaySourceContext(sourceContextCandidate[0], sourceContextCandidate[1])

        if targetDisasmAddr:
            self.disassemble(targetDisasmAddr)

        print("") # clean separation
        self.registerBuffer = []

    def flushStack(self):
        if not self.stackBuffer:
            return

        print(f"{Colors.BOLD}Stack Dump{Colors.RESET}")
        
        # 1. Reprint original lines with coloring
        def colorReplacer(match):
            valStr = match.group(0)
            try:
                val = int(valStr, 16)
                return f"{getAddrColor(val)}{valStr}{Colors.RESET}"
            except:
                return valStr

        for line in self.stackLines:
             print(re.sub(r"0x[0-9a-fA-F]+", colorReplacer, line), end='')
        
        # 2. Analyze stack for summary (silent resolution)
        funcsFound = []
        labelsFound = []

        currentFrame = 0
        frameFuncs = []
        frameLabels = []

        for val in self.stackBuffer:
            addrStr = f"0x{val:08x}"
            resolved = False
            addrColored = f"{getAddrColor(val)}{addrStr}{Colors.RESET}"

            # Check for canary/poison
            if val in [0xDEADBEEF, 0xA5A5A5A5, 0xFEFEFEFE, 0xABABABAB]:
                # Commit current frame
                if frameFuncs:
                    funcsFound.append((currentFrame, frameFuncs))
                if frameLabels:
                    labelsFound.append((currentFrame, frameLabels))
                
                frameFuncs = []
                frameLabels = []
                currentFrame += 1
                continue
            
            # Code (0x4...)
            if 0x40000000 <= val < 0x50000000:
                res = self.resolveCode(addrStr)
                if res and "??:0" not in res:
                    # Colorize Source Location
                    resDisplay = res.replace(" at ", f" at {Colors.MAGENTA}") + Colors.RESET
                    # frameFuncs.append(f"{addrColored} {resDisplay}")
                    item = f"{addrColored} {resDisplay}"
                    if not frameFuncs or frameFuncs[-1] != item: # Simple dedup consecutive
                         frameFuncs.append(item)
                    resolved = True
                else:
                    # Fallback to symbol
                    sym = findSymbol(val, self.mapSymbols)
                    if sym:
                        # frameFuncs.append(f"{addrColored} {Colors.LIGHT_BLUE}{sym}{Colors.RESET}")
                        item = f"{addrColored} {Colors.LIGHT_BLUE}{sym}{Colors.RESET}"
                        if not frameFuncs or frameFuncs[-1] != item:
                             frameFuncs.append(item)
                        resolved = True
            
            # If not code, check data (0x3... or 0x5...)
            if not resolved and ((0x30000000 <= val < 0x40000000) or (0x50000000 <= val < 0x60000000)):
                sym = findSymbol(val, self.mapSymbols)
                if sym:
                    # labelsFound.append(f"    {addrColored} -> {Colors.LIGHT_BLUE}{sym}{Colors.RESET}")
                    item = f"    {addrColored} -> {Colors.LIGHT_BLUE}{sym}{Colors.RESET}"
                    if not frameLabels or frameLabels[-1] != item: # Simple dedup consecutive
                         frameLabels.append(item)
                    resolved = True

        # Commit final frame
        if frameFuncs:
            funcsFound.append((currentFrame, frameFuncs))
        if frameLabels:
            labelsFound.append((currentFrame, frameLabels))
        
        # Summary Sections
        if funcsFound:
             print(f"\n{Colors.BOLD}Calculated Stack Trace (Functions):{Colors.RESET}")
             for frameBuffer in funcsFound:
                 # print(f"{Colors.GRAY}--- Frame {frameIdx} ---{Colors.RESET}")
                 for f in frameBuffer[1]:
                     print(f.strip())
                 if len(funcsFound) > 1 and frameBuffer != funcsFound[-1]:
                      print(f"{Colors.GRAY}---{Colors.RESET}")
        
        if labelsFound:
             print(f"\n{Colors.BOLD}Stack Symbols (Data/Labels):{Colors.RESET}")
             for frameBuffer in labelsFound:
                 # print(f"{Colors.GRAY}--- Frame {frameIdx} ---{Colors.RESET}")
                 for l in frameBuffer[1]:
                     print(l.strip())
                 if len(labelsFound) > 1 and frameBuffer != labelsFound[-1]:
                      print(f"{Colors.GRAY}---{Colors.RESET}")

        self.stackBuffer = []
        self.stackLines = []

    def processLine(self, line):
        rawLine = line
        line = line.strip()

        # State transition detection
        isRegStart = "Core" in line and "register dump:" in line
        isStackStart = "Stack memory:" in line
        isBacktrace = "Backtrace:" in line
        
        # Determine End of Blocks
        if self.state == DecoderState.IN_REGISTERS:
            # Registers end if we see Stack memory, Backtrace, or empty line
            if isStackStart or isBacktrace or not line:
                self.flushRegisters()
                self.state = DecoderState.IDLE
                # Don't return, allow fall-through to handle start of new state

        if self.state == DecoderState.IN_STACK:
            # Check for transitions inside stack block (e.g. register dump appended to line)
            if isRegStart or (not rawLine.strip() and self.state == DecoderState.IN_STACK):
                 # Current line 'rawLine' might contain stack tokens AND the start trigger
                 # We must ensure we captured the tokens from this line FIRST.
                 
                 # 1. Capture text for reprint
                 self.stackLines.append(rawLine)
                 
                 # 2. Capture hex values from THIS line if present
                 parts = line.split(':')
                 content = parts[-1] if ':' in line else line
                 hexVals = re.findall(r"(0x[0-9a-fA-F]{8})", content)
                 if hexVals:
                    for h in hexVals:
                        try:
                            # Avoid capturing register values? 
                            # Registers key-vals: MEPC : 0x... 
                            # Stack: 0x... 0x...
                            # If line has "Register dump:", it probably doesn't have stack values AFTER the text?
                            # Input: "... 0x00 0x17 Core 0 register dump:"
                            # Regex will catch 0x00 and 0x17. Good.
                            self.stackBuffer.append(int(h, 16))
                        except: pass

                 # 3. Flush now that we have everything
                 self.flushStack()
                 
                 if isRegStart:
                     self.state = DecoderState.IN_REGISTERS
                     # We consumed the line in stackLines (flushStack printed it). 
                     # Return to avoid double printing or reprocessing as IDLE
                     return 
                 else:
                     self.state = DecoderState.IDLE
                     # If empty line caused end, we consumed it.
                     return

        # State Handling
        if self.state == DecoderState.IDLE:
            if not self.interactive:
                print(rawLine, end='')

            if isRegStart:
                self.state = DecoderState.IN_REGISTERS
            elif isStackStart:
                self.state = DecoderState.IN_STACK

        elif self.state == DecoderState.IN_REGISTERS:
            # If we just entered IN_REGISTERS from IDLE above, we already printed the header.
            # But if we are continuing:
            if not isRegStart and not self.interactive: # Don't reprint header if we handled it in IDLE
                 print(rawLine, end='')
            
            # Parse registers: "Name : 0xVal"
            # PC      : 0x400d1f28  PS      : 0x00060830
            matches = list(re.finditer(r"([A-Z0-9_]+)\s*:\s*(0x[0-9a-fA-F]+)", line))
            if matches:
                for m in matches:
                    self.registerBuffer.append((m.group(1), m.group(2)))

        elif self.state == DecoderState.IN_STACK:
            self.stackLines.append(rawLine)
            # Collect hex values
            # 3ffb1d90: 0x00000000 0x00000000 ...
            
            parts = line.split(':')
            content = parts[-1] if ':' in line else line
            
            hexVals = re.findall(r"(0x[0-9a-fA-F]{8})", content)
            if hexVals:
                for h in hexVals:
                    try:
                        self.stackBuffer.append(int(h, 16))
                    except: pass

def printLegend(stream=sys.stderr):
    print(f"{Colors.BOLD}Color Legend:{Colors.RESET}", file=stream)
    print(f"  {Colors.GREEN}0x40xxxxxx{Colors.RESET} - Code (IRAM/IROM)      {Colors.MAGENTA}File:Line{Colors.RESET} - Source Location", file=stream)
    print(f"  {Colors.YELLOW}0x30xxxxxx{Colors.RESET} - Data (DRAM/DROM)      {Colors.LIGHT_BLUE}Symbol{Colors.RESET}    - Function/Variable", file=stream)
    print(f"  {Colors.CYAN}0x50xxxxxx{Colors.RESET} - External SPI RAM      {Colors.RED}Canary{Colors.RESET}    - Stack Poison/Canary", file=stream)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: \n\t{sys.argv[0]} <file.elf> [<error-stack.log>]")
        sys.exit(1)

    elfFile = sys.argv[1]

    # Tool Selection
    soc = os.environ.get('SMING_SOC', 'esp32').lower()
    if soc in ['esp32s2', 'esp32s3']:
         toolName = f"xtensa-{soc}-elf-addr2line"
    elif soc in ['esp32c3', 'esp32c2', 'esp32c6']:
         toolName = "riscv32-esp-elf-addr2line"
    else:
         toolName = "xtensa-esp32-elf-addr2line"
    
    if shutil.which(toolName) is None:
        print(f"Error: '{toolName}' not found in PATH.", file=sys.stderr)
        sys.exit(1)

    # Helper: Find nm tool
    nmToolName = toolName.replace("addr2line", "gcc-nm")
    if shutil.which(nmToolName) is None:
         nmToolName = toolName.replace("addr2line", "nm")
         if shutil.which(nmToolName) is None:
             nmToolName = None

    # SHA256 Check
    localHash = getElfSha256(elfFile)
    if localHash:
        print(f"Local ELF SHA256: {localHash[:16]}...", file=sys.stderr)

    # Load symbols: Prefer ELF (nm), fallback to Map
    symbols = loadElfSymbols(elfFile, nmToolName)
    if not symbols:
         symbols = loadMapSymbols(elfFile)

    # Helper: Find objdump tool
    objdumpToolName = toolName.replace("addr2line", "objdump")
    if shutil.which(objdumpToolName) is None:
         objdumpToolName = None

    # Input Stream Setup
    inputStream = sys.stdin
    interactive = False
    usePager = False

    if len(sys.argv) > 2:
        try:
            inputStream = open(sys.argv[2], 'r')
        except Exception as e:
            print(f"Error opening log file: {e}")
            sys.exit(1)
    else:
        # Check if stdin is a TTY (interactive mode)
        interactive = sys.stdin.isatty()
        usePager = interactive

    # Setup Pager Capture
    captureBuffer = None
    originalStdout = sys.stdout
    
    if usePager:
        captureBuffer = io.StringIO()
        sys.stdout = captureBuffer

    printLegend(sys.stdout if usePager else sys.stderr)

    # In pager mode, we want to capture (echo) the input lines into the buffer
    # so the final output contains everything. Thus interactive=False.
    decoder = CrashDecoder(elfFile, toolName, symbols, soc, objdumpToolName, interactive=False)

    try:
        if interactive:
            print(f"{Colors.BOLD}Decode stack trace: Paste stack trace here (Ctrl+D to finish){Colors.RESET}", file=sys.stderr)
        
        for line in inputStream:
            # SHA256 extraction from stream (stateless check)
            if localHash and "ELF file SHA256:" in line:
                m = re.search(r"SHA256:\s*([0-9a-fA-F]+)", line)
                if m:
                    remoteHash = m.group(1)
                    if not localHash.startswith(remoteHash.lower()):
                        # If paginating, this warning goes to the buffer
                        print(f"\n{Colors.RED}{Colors.BOLD}WARNING: ELF Checksum Mismatch!{Colors.RESET}")
                        print(f"{Colors.RED}Dump:  {remoteHash}{Colors.RESET}")
                        print(f"{Colors.RED}Local: {localHash}{Colors.RESET}\n")

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
        if inputStream is not sys.stdin:
            inputStream.close()

    if usePager and captureBuffer:
         sys.stdout = originalStdout # Restore
         output = captureBuffer.getvalue()
         
         if shutil.which("less"):
             try:
                 # -R: ANSI colors, -F: Exit if one screen, -X: No init (keep on screen on exit)
                 subprocess.run(["less", "-R", "-F", "-X"], input=output, text=True)
             except Exception:
                 print(output)
         else:
             print(output)

if __name__ == "__main__":
    main()
