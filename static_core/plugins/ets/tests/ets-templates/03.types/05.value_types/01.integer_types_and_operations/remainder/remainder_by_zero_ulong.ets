/*---
desc: check remainder of division by zero for unsigned long integer operands
---*/

function main(): void {
    try {
        const a: ulong = 1
        const b: ulong = 0
        const result: ulong = a % b
        assert(false)
    } catch (e: ArithmeticException) {
    }
}

