/*---
desc: >-
  If the left-hand operand expression is a field access expression e.f (§7.11), possibly enclosed in one or more pairs of parentheses, then:
  - First, the expression e is evaluated. If evaluation of e completes abruptly, the assignment expression completes abruptly for the same reason.
  - Next, the right hand operand is evaluated. If evaluation of the right hand expression completes abruptly, the assignment expression completes abruptly for the same reason.
  - Finally, the variable denoted by e.f is assigned the value of the right hand operand as computed above.
params: normal completion, instance
---*/

let result: string = ""

class A {
    public fld: string = ""
}

function getObj(): A {
    result = result + "1"
    return new A()
}

function getVal(): string {
    result = result + "2"
    return "abc"
}

function main(): int {
    result = "";  // reset due to jit mode warmup

    let s: string = getObj().fld = getVal()
    if (result != "12" || s != "abc") {
        return 1
    }
    return 0
}
