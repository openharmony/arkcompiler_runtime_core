/*---
desc: check remainder of division by zero for unsigned byte operands
---*/

function main(): void {
    try {
        const a: ubyte = 1
        const b: ubyte = 0
        const result: ubyte = a % b
        assert(false)
    } catch (e: ArithmeticException) {
    }
}

