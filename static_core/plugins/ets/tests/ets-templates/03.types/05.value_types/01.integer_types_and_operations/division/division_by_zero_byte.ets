/*---
desc: check division by zero for byte operand
---*/

function main(): void {
    try {
        const a: byte = 1
	    const b: byte = 0
	    const result: byte = a / b
	    assert(false)
    } catch (e: ArithmeticException) {
    }
}

