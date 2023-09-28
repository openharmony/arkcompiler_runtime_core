/*---
desc: check division by zero for short integer operand
---*/

function main(): void {
    try {
        const a: short = 1
	    const b: short = 0
	    const result: short = a / b
	    assert(false)
    } catch (e: ArithmeticException) {
    }
}

