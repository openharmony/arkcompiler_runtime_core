/*---
desc: check remainder of division by zero for integer operands
---*/

function main(): void {
    try {
        const a: int = 1
        const b: int = 0
        const result: int = a % b
        assert(false)
    } catch (e: ArithmeticException) {
    }
}

