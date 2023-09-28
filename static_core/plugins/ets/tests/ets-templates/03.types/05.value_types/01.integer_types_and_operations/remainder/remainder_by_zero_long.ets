/*---
desc: check remainder of division by zero for long integer operands
---*/

function main(): void {
    try {
        const a: long = 1
        const b: long = 0
        const result: long = a % b
        assert(false)
    } catch (e: ArithmeticException) {
    }
}

