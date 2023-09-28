/*---
desc: check division by zero for long integer operand
---*/

function main(): void {
    try {
        const a: long = 1
	    const b: long = 0
	    const result: long = a / b
	    assert(false)
    } catch (e: ArithmeticException) {
    }
}

