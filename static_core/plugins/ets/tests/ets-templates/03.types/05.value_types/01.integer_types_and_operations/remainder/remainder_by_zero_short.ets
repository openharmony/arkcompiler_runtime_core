/*---
desc: check remainder of division by zero for short integer operands
---*/

function main(): void {
    try {
        const a: short = 1
        const b: short = 0
        const result: short = a % b
        assert(false)
    } catch (e: ArithmeticException) {
    }
}

