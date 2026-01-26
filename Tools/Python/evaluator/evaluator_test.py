#!/usr/bin/env python3
import os
import sys

# 1. Get SMING_HOME from environment
sming_home = os.environ.get("SMING_HOME")
if not sming_home:
    sys.exit("Error: SMING_HOME environment variable is not set.")

# 2. Construct the absolute path to the Python tools directory
# Based on your structure: $SMING_HOME/../Tools/Python
tools_path = os.path.abspath(os.path.join(sming_home, "..", "Tools", "Python"))

# 3. Add that directory to the Python search path
if tools_path not in sys.path:
    sys.path.append(tools_path)

# 4. Now you can import the Evaluator class from evaluator.py
try:
    from evaluator import Evaluator
except ImportError as e:
    sys.exit(f"Error: Could not find evaluator.py in {tools_path}. {e}")

def run_full_suite():
    evaluator = Evaluator()

    def get_variable(name: str):
        if name == 'NUMERIC_VAR':
            return 123
        return evaluator._get_variable(name)

    evaluator.get_variable = get_variable

    # Environment Setup
    os.environ.update({
        "CPU_COUNT": "4",
        "ARCH": "xtensa",
        "CHIP": "esp32s3",
        "RAM_SIZE": "0x80000", # 512KB
        "DEBUG_LEVEL": "2",
        "IS_RELEASE": "False",
        "FEATURES": "wifi,ble,mesh"
    })

    # Format: (Expression, Expected, Category)
    test_cases = [
        # --- Arithmetic & Bitwise ---
        ("CPU_COUNT * 2", 8, "Basic Arithmetic"),
        ("RAM_SIZE / 1024", 512.0, "Hex Handling"),
        ("1 << CPU_COUNT", 16, "Bit Shift"),
        ("~0 & 0xFF", 255, "Bitwise Invert & AND"),
        ("align_up(15, 8)", 16, "Custom Function"),
        ("NUMERIC_VAR * 2", 123 * 2, "Custom variable"),

        # --- Multi-Argument & Custom Math ---
        ("max(1, 5, 2)", 5, "Variadic Function (max)"),
        ("min(10, CPU_COUNT, 8)", 4, "Variadic with Env Var"),
        ("align_up(13, 4)", 16, "Alignment (Already aligned)"),
        ("align_up(17, 8)", 24, "Alignment (Cross boundary)"),
        ("align_up(32, 16)", 32, "Alignment (No change needed)"),

        # --- C-Style Logical Aliases ---
        ("True if CPU_COUNT == 4 && ARCH == 'xtensa' else False", True, "AND alias (&&)"),
        ("True if ARCH == 'arm' || ARCH == 'xtensa' else False", True, "OR alias (||)"),
        ("True if !(ARCH == 'arm') else False", True, "NOT alias (!)"),
        ("True if ARCH != 'arm' else False", True, "NOT EQUAL (!=)"),

        # --- Substring & Membership ---
        ("'esp' in CHIP", True, "Substring match (in)"),
        ("'mesh' in FEATURES", True, "Env string search"),
        ("True if '32s' in CHIP && 'wifi' in FEATURES else False", True, "Nested Logic+Member"),
        ("'riscv' not in ARCH", True, "Negative Member"),

        # --- Formatting ---
        ("f'0x{CPU_COUNT:02x}'", '0x04', "Hex formatted value"),
        ("f'{CPU_COUNT:>2}'", ' 4', "Padded string"),
        ("f'{CPU_COUNT:.2f}'", '4.00', "Float string"),

        # --- Nested Ternary & Math ---
        ("pow2(CPU_COUNT) if CPU_COUNT > 2 else 0", 16, "Function in Ternary"),
        ("8 if CHIP == 'esp32' else (16 if CHIP == 'esp32s3' else 32)", 16, "Nested Else-If"),
        ("(1 if !IS_RELEASE else 0)", 1, "Bool Env + NOT (!)"),

        # --- Error Handling ---
        ("UNDEFINED_VAR + 1", "Error: Variable 'UNDEFINED_VAR' is not defined.", "Missing Var"),
        ("pow2(2, 4, 6)", "Error: Function 'pow2' expects 1 arguments (got 3)", "Param Mismatch"),

        # --- Code Injection (Security Audit) ---
        ("__import__('os').system('ls')", "Error: Unsupported function call type: Attribute", "Injection: RCE"),
        ("os.popen('whoami')", "Error: Unsupported function call type: Attribute", "Injection: Attribute"),
        ("open('/etc/passwd')", "Error: Forbidden function call: open", "Injection: File Access"),
        ("().__class__.__mro__", "Error: Unsupported syntax: Attribute", "Injection: Jailbreak"),
        ("1; import os", "Error: invalid syntax", "Injection: Multi-stmt"),
    ]

    print(f"{'Category':<30} | {'Expression':<60} | {'Result'}")
    print("-" * 110)

    passed = 0
    for expr, expected, cat in test_cases:
        try:
            # Logic: If 'expected' is an error string, check if it's contained in the 'actual' result
            actual = evaluator.run(expr)
            success = (actual == expected)
        except Exception as e:
            success = f'Error: {e}'.startswith(expected)
            actual = repr(e)

        if success:
            passed += 1
            status = "\033[92mPASS\033[0m"
        else:
            status = f"\033[91mFAIL\033[0m (Got: {actual})"

        print(f"{cat:<30} | {expr:<60} | {status}")

    print("-" * 110)
    print(f"Final Score: {passed}/{len(test_cases)}")

if __name__ == "__main__":
    run_full_suite()
