/*---
desc: check remainder of division by zero for unsigned short integer operands
---*/

function main(): void {
    try {
        const a: ushort = 1
	    const b: ushort = 0
	    const result: ushort = a % b
	    assert(false)
    } catch (e: ArithmeticException) {
    }
}

