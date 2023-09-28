/*---
desc: check remainder of division by zero for unsigned integer operands
---*/

function main(): void {
    try {
        const a: uint = 1
        const b: uint = 0
        const result: uint = a % b
        assert(false)
    } catch (e: ArithmeticException) {
    }
}

