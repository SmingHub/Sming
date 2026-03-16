#!/usr/bin/env python
########################################################
#
#  Stack Trace Decoder (ESP8266 - Stateful)
#
# Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
# Created 2015 by Skurydin Alexey
# http://github.com/SmingHub/Sming
# All files of the Sming Core are provided under the LGPL v3 license.
#
########################################################
import sys
import os
import re
import subprocess
import hashlib
import shutil
import io
from enum import Enum, auto

class Colors:
    RESET      = "\033[0m"
    BOLD       = "\033[1m"
    RED        = "\033[31m"
    GREEN      = "\033[32m"
    YELLOW     = "\033[33m"
    MAGENTA    = "\033[35m"
    GRAY       = "\033[90m"
    LIGHT_BLUE = "\033[94m"

CANARY_VALUES = {0xDEADBEEF, 0xA5A5A5A5, 0xFEFEFEFE, 0xABABABAB, 0x55AA55AA, 0xAA55AA55}

def getAddrColor(val):
    """Return ANSI color based on ESP8266 memory region."""
    if val in CANARY_VALUES:
        return Colors.RED
    if 0x40000000 <= val < 0x40400000:   # IRAM/IROM (Code)
        return Colors.GREEN
    if 0x3ffe0000 <= val < 0x40000000:   # DRAM (Data)
        return Colors.YELLOW
    return Colors.RESET

def colorizeHex(line):
    """Colorize 0x-prefixed hex values in a line using getAddrColor."""
    def replacer(m):
        s = m.group(0)
        try:
            return f"{getAddrColor(int(s, 16))}{s}{Colors.RESET}"
        except Exception:
            return s
    return re.sub(r'0x[0-9a-fA-F]+', replacer, line)

XTENSA_EXCEPTIONS = {
    0:  "Illegal Instruction",
    1:  "Syscall",
    2:  "Instruction Fetch Error",
    3:  "Load/Store Error",
    4:  "Level 1 Interrupt",
    5:  "Alloca Exception",
    6:  "Integer Divide by Zero",
    28: "Load Prohibited",
    29: "Store Prohibited",
}

def isCodeAddr(val):
    return 0x40000000 <= val < 0x40400000

def isDataAddr(val):
    return 0x3ffe0000 <= val < 0x40000000

def getElfSha256(filepath):
    try:
        h = hashlib.sha256()
        with open(filepath, "rb") as f:
            for block in iter(lambda: f.read(4096), b""):
                h.update(block)
        return h.hexdigest()
    except Exception:
        return None

def loadElfSymbols(elfPath, nmTool):
    if not nmTool or not shutil.which(nmTool):
        return []
    symbols = []
    print(f"Loading symbols from ELF using {nmTool}...", file=sys.stderr)
    try:
        result = subprocess.run([nmTool, '-n', elfPath],
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                text=True, check=False)
        for line in result.stdout.splitlines():
            parts = line.split()
            if len(parts) >= 3:
                try:
                    symbols.append((int(parts[0], 16), parts[2]))
                except ValueError:
                    pass
        print(f"Loaded {len(symbols)} symbols from ELF.", file=sys.stderr)
    except Exception as e:
        print(f"Error running nm: {e}", file=sys.stderr)
    return symbols

def loadMapSymbols(elfPath):
    mapPath = os.path.splitext(elfPath)[0] + ".map"
    if not os.path.exists(mapPath):
        return []
    symbols = []
    mapRegex = re.compile(r"^\s+(0x[0-9a-fA-F]{8,})\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*$")
    try:
        with open(mapPath, 'r') as f:
            for line in f:
                if "0x" not in line:
                    continue
                m = mapRegex.match(line)
                if m:
                    symbols.append((int(m.group(1), 16), m.group(2)))
        symbols.sort(key=lambda x: x[0])
        print(f"Loaded {len(symbols)} symbols from map file.", file=sys.stderr)
    except Exception as e:
        print(f"Warning: Failed to parse map file: {e}", file=sys.stderr)
    return symbols

def findSymbol(address, symbols):
    if not symbols:
        return None
    best = None
    for symAddr, symName in symbols:
        if symAddr > address:
            break
        best = (symAddr, symName)
    if best:
        offset = address - best[0]
        if offset < 8192:
            return f"{best[1]}+{offset}" if offset else best[1]
    return None

class DecoderState(Enum):
    IDLE         = auto()
    IN_REGISTERS = auto()
    IN_STACK     = auto()

class CrashDecoder:
    def __init__(self, elfFile, toolPath, mapSymbols, objdumpTool=None):
        self.elfFile     = elfFile
        self.toolPath    = toolPath
        self.mapSymbols  = mapSymbols
        self.objdumpTool = objdumpTool
        self.state           = DecoderState.IDLE
        self.addr2lineProc   = None
        self.registerBuffer  = []   # list of (name, addrStr)
        self.stackBuffer     = []   # list of int
        self.stackLines      = []   # list of raw text lines

        try:
            self.addr2lineProc = subprocess.Popen(
                [toolPath, '-aifC', '-e', elfFile],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL,
                text=True,
                bufsize=1
            )
        except Exception as e:
            print(f"Error starting {toolPath}: {e}", file=sys.stderr)
            sys.exit(1)

    def close(self):
        if self.addr2lineProc:
            self.addr2lineProc.terminate()

    def resolveCode(self, addrStr):
        if not self.addr2lineProc:
            return None
        try:
            self.addr2lineProc.stdin.write(f"{addrStr}\n0x00000000\n")
            self.addr2lineProc.stdin.flush()
            outputLines = []
            foundEcho = False
            while True:
                line = self.addr2lineProc.stdout.readline()
                if not line:
                    break
                line = line.strip()
                if line == "0x00000000":
                    self.addr2lineProc.stdout.readline()  # consume ??
                    self.addr2lineProc.stdout.readline()  # consume ??:0
                    break
                if line.lower() == addrStr.lower():
                    foundEcho = True
                    continue
                if foundEcho:
                    outputLines.append(line)
            if len(outputLines) >= 2:
                results = []
                seen = set()
                for i in range(0, len(outputLines), 2):
                    if i + 1 < len(outputLines):
                        func = outputLines[i].strip()
                        loc  = re.sub(r'\s*\(discriminator \d+\)', '', outputLines[i + 1].strip())
                        if func != "??" and loc != "??:0" and func not in seen:
                            seen.add(func)
                            results.append(f"{func} at {loc}")
                return " | ".join(results) if results else None
        except Exception:
            pass
        return None

    def disassemble(self, addr):
        if not self.objdumpTool:
            return
        cmd = [self.objdumpTool, '-d',
               '--start-address', f'0x{addr:x}',
               '--stop-address',  f'0x{addr + 24:x}',
               self.elfFile]
        try:
            res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
            if res.returncode == 0:
                print(f"\n  {Colors.BOLD}Disassembly around {getAddrColor(addr)}0x{addr:x}{Colors.RESET}:")
                for line in res.stdout.splitlines():
                    if re.match(r'\s*[0-9a-fA-F]+:', line):
                        print(f"    {Colors.GRAY}{line}{Colors.RESET}")
        except Exception:
            pass

    def displaySourceContext(self, filePath, lineNum):
        if not filePath or lineNum <= 0:
            return
        if not os.path.exists(filePath) and not os.path.exists(os.path.abspath(filePath)):
            print(f"\n  {Colors.GRAY}(source not available locally: {filePath}){Colors.RESET}")
            return
        try:
            with open(filePath, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
            start = max(1, lineNum - 2)
            end   = min(len(lines), lineNum + 2)
            print(f"\n  {Colors.BOLD}Context in {Colors.MAGENTA}{os.path.basename(filePath)}{Colors.RESET}:")
            for i in range(start, end + 1):
                prefix = " >" if i == lineNum else "  "
                color  = Colors.GREEN if i == lineNum else Colors.GRAY
                print(f"   {prefix} {i:4d}: {color}{lines[i-1].rstrip()}{Colors.RESET}")
        except Exception:
            pass

    def flushRegisters(self):
        if not self.registerBuffer:
            return
        print(f"\n{Colors.BOLD}Register decode{Colors.RESET}")
        pcAddr          = None
        sourceCandidate = None

        for name, addrStr in self.registerBuffer:
            try:
                val = int(addrStr, 16)
            except ValueError:
                continue

            if name == "EXCCAUSE":
                desc = XTENSA_EXCEPTIONS.get(val, f"Unknown (code {val})")
                print(f"  {'EXCCAUSE':8}: {Colors.RED}{addrStr}{Colors.RESET}  ({Colors.RED}{desc}{Colors.RESET})")
                continue

            if name == "PC":
                pcAddr = val

            res = None
            if isCodeAddr(val):
                res = self.resolveCode(addrStr)
                if not res or "??:0" in res:
                    sym = findSymbol(val, self.mapSymbols)
                    if sym:
                        res = sym
                if name == "PC" and res and " at " in res:
                    m = re.search(r' at (.*?):(\d+)', res)
                    if m:
                        sourceCandidate = (m.group(1), int(m.group(2)))
            elif isDataAddr(val):
                sym = findSymbol(val, self.mapSymbols)
                if sym:
                    res = sym

            addrColored = f"{getAddrColor(val)}{addrStr}{Colors.RESET}"
            if res and res != "?? at ??:0":
                if " at " in res:
                    res = res.replace(" at ", f" at {Colors.MAGENTA}").replace(" | ", f"{Colors.RESET} | ") + Colors.RESET
                else:
                    res = f"{Colors.LIGHT_BLUE}{res}{Colors.RESET}"
                print(f"  {name:8}: {addrColored}  ({res})")
            else:
                print(f"  {name:8}: {addrColored}")

        if sourceCandidate:
            self.displaySourceContext(sourceCandidate[0], sourceCandidate[1])
        if pcAddr:
            if isCodeAddr(pcAddr):
                self.disassemble(pcAddr)
            else:
                region = "Data (DRAM)" if isDataAddr(pcAddr) else "unknown region"
                print(f"\n  {Colors.RED}PC (0x{pcAddr:08x}) is in {region} — execution jumped to non-code memory.{Colors.RESET}")
                print(f"  {Colors.RED}No code can be disassembled here. Likely a wild pointer or stack corruption.{Colors.RESET}")

        print("")
        self.registerBuffer = []

    def flushStack(self):
        if not self.stackBuffer:
            return
        print(f"{Colors.BOLD}Stack Dump{Colors.RESET}")

        def colorReplacer(match):
            valStr = match.group(0)
            try:
                return f"{getAddrColor(int(valStr, 16))}{valStr}{Colors.RESET}"
            except Exception:
                return valStr

        for line in self.stackLines:
            print(re.sub(r'\b[0-9a-fA-F]{8}\b', colorReplacer, line), end='')

        funcsFound  = []
        labelsFound = []
        frameFuncs  = []
        frameLabels = []

        for val in self.stackBuffer:
            addrStr    = f"0x{val:08x}"
            addrColored = f"{getAddrColor(val)}{addrStr}{Colors.RESET}"

            if val in CANARY_VALUES:
                if frameFuncs:  funcsFound.append(frameFuncs)
                if frameLabels: labelsFound.append(frameLabels)
                frameFuncs, frameLabels = [], []
                continue

            if isCodeAddr(val):
                res = self.resolveCode(addrStr)
                if res and "??:0" not in res:
                    item = f"{addrColored} {res.replace(' at ', f' at {Colors.MAGENTA}').replace(' | ', f'{Colors.RESET} | ') + Colors.RESET}"
                    if not frameFuncs or frameFuncs[-1] != item:
                        frameFuncs.append(item)
                else:
                    sym = findSymbol(val, self.mapSymbols)
                    if sym:
                        item = f"{addrColored} {Colors.LIGHT_BLUE}{sym}{Colors.RESET}"
                        if not frameFuncs or frameFuncs[-1] != item:
                            frameFuncs.append(item)
            elif isDataAddr(val):
                sym = findSymbol(val, self.mapSymbols)
                if sym:
                    item = f"    {addrColored} -> {Colors.LIGHT_BLUE}{sym}{Colors.RESET}"
                    if not frameLabels or frameLabels[-1] != item:
                        frameLabels.append(item)

        if frameFuncs:  funcsFound.append(frameFuncs)
        if frameLabels: labelsFound.append(frameLabels)

        if funcsFound:
            print(f"\n{Colors.BOLD}Calculated Stack Trace (Functions):{Colors.RESET}")
            for i, frame in enumerate(funcsFound):
                for f in frame:
                    print(f.strip())
                if i < len(funcsFound) - 1:
                    print(f"{Colors.GRAY}---{Colors.RESET}")

        if labelsFound:
            print(f"\n{Colors.BOLD}Stack Symbols (Data/Labels):{Colors.RESET}")
            for i, frame in enumerate(labelsFound):
                for l in frame:
                    print(l.strip())
                if i < len(labelsFound) - 1:
                    print(f"{Colors.GRAY}---{Colors.RESET}")

        self.stackBuffer = []
        self.stackLines  = []

    def processLine(self, line):
        rawLine  = line
        stripped = line.strip()

        # Exception header: "***** Fatal exception N (NAME)"
        m = re.match(r'\*+\s*Fatal exception (\d+)\s*\(([^)]+)\)', stripped)
        if m:
            code = int(m.group(1))
            name = m.group(2)
            desc = XTENSA_EXCEPTIONS.get(code, name)
            print(f"{Colors.RED}{Colors.BOLD}Fatal exception {code}: {desc}{Colors.RESET}")
            # Buffer as EXCCAUSE so it appears in the register decode section
            self.registerBuffer.append(("EXCCAUSE", f"0x{code:08x}"))
            return

        # PC/SP/excvaddr line
        m = re.match(r'pc=(0x[0-9a-fA-F]+)\s+sp=(0x[0-9a-fA-F]+)\s+excvaddr=(0x[0-9a-fA-F]+)', stripped)
        if m:
            print(colorizeHex(rawLine), end='')
            self.registerBuffer += [("PC", m.group(1)), ("SP", m.group(2)), ("EXCVADDR", m.group(3))]
            self.state = DecoderState.IN_REGISTERS
            return

        if self.state == DecoderState.IN_REGISTERS:
            # PS/SAR/VPRI line
            m = re.match(r'ps=(0x[0-9a-fA-F]+)\s+sar=(0x[0-9a-fA-F]+)\s+vpri=(0x[0-9a-fA-F]+)', stripped)
            if m:
                print(colorizeHex(rawLine), end='')
                self.registerBuffer += [("PS", m.group(1)), ("SAR", m.group(2)), ("VPRI", m.group(3))]
                return

            # r00..r15 register lines
            regMatches = list(re.finditer(r'\b(r\d{1,2}):\s*(0x[0-9a-fA-F]+)=', stripped))
            if regMatches:
                print(colorizeHex(rawLine), end='')
                for rm in regMatches:
                    self.registerBuffer.append((rm.group(1).upper(), rm.group(2)))
                return

            # End of register section
            self.flushRegisters()
            self.state = DecoderState.IDLE

        # Stack dump marker
        if re.search(r'[Ss]tack dump:', stripped):
            print(rawLine, end='')
            self.state = DecoderState.IN_STACK
            return

        if self.state == DecoderState.IN_STACK:
            # Actual stack data: "3ffff350:  3ffff628 00000001 ..."
            m = re.match(r'([0-9a-fA-F]{8}):\s+((?:[0-9a-fA-F]{8}\s*)+)', stripped)
            if m:
                self.stackLines.append(rawLine)
                for h in re.findall(r'[0-9a-fA-F]{8}', m.group(2)):
                    try:
                        self.stackBuffer.append(int(h, 16))
                    except Exception:
                        pass
                return

            # Blank line → end of stack, but only once we've seen actual data.
            # Blank lines before the first data line are noise (e.g. between the
            # "make decode-stacktrace" instructions and the "===" delimiter).
            if not stripped:
                if self.stackBuffer:
                    self.flushStack()
                    self.state = DecoderState.IDLE
                # else: blank line before any data - stay in IN_STACK
                return

            # Pass-through noise (delimiter lines, instructions)
            print(rawLine, end='')
            return

        # Default pass-through
        print(rawLine, end='')


def printLegend(stream=sys.stderr):
    print(f"{Colors.BOLD}Color Legend:{Colors.RESET}", file=stream)
    print(f"  {Colors.GREEN}0x40xxxxxx{Colors.RESET} - Code (IRAM/IROM)      {Colors.MAGENTA}File:Line{Colors.RESET} - Source Location", file=stream)
    print(f"  {Colors.YELLOW}0x3fxxxxxx{Colors.RESET} - Data (DRAM)           {Colors.LIGHT_BLUE}Symbol{Colors.RESET}    - Function/Variable", file=stream)
    print(f"  {Colors.RED}Exception{Colors.RESET}  - Fatal exception       {Colors.RED}Canary{Colors.RESET}    - Stack Poison/Canary", file=stream)


def main():
    if len(sys.argv) < 2:
        print(f"Usage: \n\t{sys.argv[0]} <file.elf> [<error-stack.log>]")
        sys.exit(1)

    elfFile  = sys.argv[1]

    # Architecture guard
    arch = os.environ.get('SMING_ARCH', '')
    if arch and arch.lower() != 'esp8266':
        print(f"Error: SMING_ARCH='{arch}' but this is the ESP8266 decoder.", file=sys.stderr)
        print(f"Use the {arch} decoder instead (Sming/Arch/{arch}/Tools/decode-stacktrace.py).", file=sys.stderr)
        sys.exit(1)

    toolName = "xtensa-lx106-elf-addr2line"

    if shutil.which(toolName) is None:
        print(f"Error: '{toolName}' not found in PATH.", file=sys.stderr)
        sys.exit(1)

    nmToolName = "xtensa-lx106-elf-gcc-nm"
    if shutil.which(nmToolName) is None:
        nmToolName = "xtensa-lx106-elf-nm"
        if shutil.which(nmToolName) is None:
            nmToolName = None

    objdumpToolName = "xtensa-lx106-elf-objdump"
    if shutil.which(objdumpToolName) is None:
        objdumpToolName = None

    localHash = getElfSha256(elfFile)
    if localHash:
        print(f"Local ELF SHA256: {localHash[:16]}...", file=sys.stderr)

    symbols = loadElfSymbols(elfFile, nmToolName)
    if not symbols:
        symbols = loadMapSymbols(elfFile)

    inputStream = sys.stdin
    usePager    = False

    if len(sys.argv) > 2:
        try:
            inputStream = open(sys.argv[2], 'r')
        except Exception as e:
            print(f"Error opening log file: {e}")
            sys.exit(1)
    else:
        usePager = sys.stdin.isatty()

    captureBuffer  = None
    originalStdout = sys.stdout

    if usePager:
        captureBuffer = io.StringIO()
        sys.stdout    = captureBuffer

    printLegend(sys.stdout if usePager else sys.stderr)

    decoder = CrashDecoder(elfFile, toolName, symbols, objdumpToolName)

    try:
        if usePager:
            print(f"{Colors.BOLD}Decode stack trace: Paste stack trace here (Ctrl+D to finish){Colors.RESET}", file=sys.stderr)

        for line in inputStream:
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
        sys.stdout = originalStdout
        output = captureBuffer.getvalue()
        if shutil.which("less"):
            try:
                subprocess.run(["less", "-R", "-F", "-X"], input=output, text=True)
            except Exception:
                print(output)
        else:
            print(output)


if __name__ == "__main__":
    main()
