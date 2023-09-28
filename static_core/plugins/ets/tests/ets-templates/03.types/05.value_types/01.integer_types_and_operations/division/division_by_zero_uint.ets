/*---
desc: check division by zero for unsigned integer operand
---*/

function main(): void {
    try {
        const a: uint = 1
	    const b: uint = 0
	    const result: uint = a / b
	    assert(false)
    } catch (e: ArithmeticException) {
    }
}
