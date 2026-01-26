# Sming Evaluator DSL Specification

`Evaluator` implements a DSL to allow computed values for various settings at compile time.

The intended purpose is to enable safe parsing of expressions involving environment variables.

A simplistic evaluation can be done using `eval()` but that has security implications,
and accessing environment variables is quite clunky.

## 2. Language Syntax

### 2.1 Literals and Types

* **Integers & Floats:** Standard numeric literals (e.g., 100, 3.14).
* **Hexadecimal:** Environment variables starting with 0x are automatically cast to integers.
* **Strings:** Single or double-quoted literals (e.g., 'esp32', "enabled") and formatted-strings (e.g. "f'{value:02x}'")
* **Booleans:** Case-sensitive True and False keywords.

### 2.2 Arithmetic & Bitwise

| Operator | Description | Example |
| :--- | :--- | :--- |
| + , - | Addition and Subtraction | MEM_BASE + 0x100 |
| * , / | Multiplication and Division | TOTAL_SIZE / 4 |
| << , >> | Bit Shifting | 1 << CPU_COUNT |
| & , | | Bitwise AND and OR | FLAGS | 0x01 |
| ~ | Bitwise Invert | ~(MASK) & 0xFF |
| ** | Exponentiation | 2 ** 8 |
| - (unary) | Negation | -1 * OFFSET |

### 2.3 Logical Aliases

The evaluator pre-processes C-style tokens into Python logical keywords:

| DSL Token | Internal Logical Node | Description |
| :--- | :--- | :--- |
| && | and | Logical AND |
| || | or | Logical OR |
| ! | not | Logical Negation |
| != | != | Not Equal |

### 2.4 Comparison & Membership

| Operator | Description | Example |
| :--- | :--- | :--- |
| == | Equality | CPU_COUNT == 4 |
| > , < | Greater/Less Than | BAUD > 9600 |
| >= , <= | Greater/Less or Equal | VERSION >= 2.0 |
| in | Substring Match | 'esp' in CHIP |
| not in | Negative Substring Match | 'beta' not in VERSION |

---

## 3. Control Flow (Ternary Logic)

The DSL uses the Python ternary structure. An else clause is mandatory.

**Syntax:** [result_if_true] if [condition] else [result_if_false]

**Example:**

```text
"HIGH_SPEED" if (BAUD >= 115200 || FORCE_FAST == "1") else "LOW_SPEED"
```

## 4. Built-in Functions

The following whitelisted functions are available.
Function calls are validated for correct argument counts via signature inspection before execution.

* **pow2(x)**: Returns 2 raised to the power of x.
* **log2(x)**: Returns the base-2 logarithm of x.
* **align_up(val, align)**: Rounds val up to the next align boundary.
* **min(...)**: Returns the minimum value of the arguments.
* **max(...)**: Returns the maximum value of the arguments.

Applications may extend this by modifying the `functions` property.


## 5. Environment Variables & Truthiness

Variables are pulled from os.environ.
When used in a conditional if or with !, the following "Truthiness" rules apply to strings:

* **Falsy:** "0", "False", "no", "off", or "" (empty string).
* **Truthy:** Any other value (e.g., "1", "True", "esp32").

## 6. Security Guardrails

The Evaluator prevents common Python injection attacks through a strict "Fail-Closed" model:

1. **No Attribute Access:** The "." operator is forbidden.
   Attempting to call methods (e.g., os.system()) or access object properties (e.g., obj.__class__)
   results in an "Unsupported function call type: Attribute" or "Unsupported syntax: Attribute" error.
2. **No Multi-Statement Logic:** Semicolons (;) and newlines are rejected by the parser (mode='eval').
3. **No Imports:** Keywords like import or from are not supported in the expression grammar.
4. **Whitelisted Nodes:** Only specific AST nodes (BinOp, UnaryOp, BoolOp, Compare, IfExp, Call, Constant, Name) are evaluated.

## Example expressions

```text
"True if 'esp32' in SMING_SOC else False"
"'HIGH_SPEED' if COM_SPEED > 115200 else 'LOW_SPEED'"
"f'{RBOOT_ROM0_ADDR:08x}'"
```

### Exceptions

Errors will result in an exception, for example:

* Unsupported function call type: Attribute: Triggered by attempting to use a "." in a function call.
* Forbidden function call: open: Triggered by attempting to call a function not in the whitelist.
* Function `pow2` expects 1 arguments (got 3): Triggered by incorrect argument counts.
* Variable 'VAR' is not defined: Does not exist in environment and not provided via `get_variable` callback.
