/*---
desc: check division by zero for integer operand
---*/

function main(): void {
    try {
        const a: int = 1
	    const b: int = 0
	    const result: int = a / b
	    assert(false)
    } catch (e: ArithmeticException) {
    }
}

