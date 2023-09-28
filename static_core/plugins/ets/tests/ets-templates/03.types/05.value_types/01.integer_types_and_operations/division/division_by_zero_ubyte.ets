/*---
desc: check division by zero for unsigned byte operand
---*/

function main(): void {
    try {
        const a: ubyte = 1
	    const b: ubyte = 0
	    const result: ubyte = a / b
	    assert(false)
    } catch (e: ArithmeticException) {
    }
}

