/*---
desc: >-
  If the left-hand operand expression is a field access expression e.f (§7.11), possibly enclosed in one or more pairs of parentheses, then:
  - First, the expression e is evaluated. If evaluation of e completes abruptly, the assignment expression completes abruptly for the same reason.
  - Next, the right hand operand is evaluated. If evaluation of the right hand expression completes abruptly, the assignment expression completes abruptly for the same reason.
  - Finally, the variable denoted by e.f is assigned the value of the right hand operand as computed above.
params: abrupt completion, left-hand operand, static
tags: [no-warmup]
---*/

let result: string = ""

class A {
    public static fld: string = new Int(1/0).toString()  // get ArithmeticException
}

function getVal(): string {
    result = result + "1"
    return "abc"
}

function main(): int {
    let s: string = ""
    try {
        s = ((A.fld)) = getVal()
    } catch (e: ArithmeticException) {
        if (result == "1" && s == "") {
            return 0
        }
    }
    return 1
}
