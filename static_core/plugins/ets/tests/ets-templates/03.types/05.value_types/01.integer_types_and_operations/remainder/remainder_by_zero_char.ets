/*---
desc: check remainder of division by zero for char operands
---*/

function main(): void {
    try {
        const a: char = 1
        const b: char = 0
        const result: char = a % b
        assert(false)
    } catch (e: ArithmeticException) {
    }
}

