/*---
desc: check division by zero for unsigned long operand
---*/

function main(): void {
    try {
        const a: ulong = 1
	    const b: ulong = 0
	    const result: ulong = a / b
	    assert(false)
    } catch (e: ArithmeticException) {
    }
}

