/*---
desc: A catch clause contains a block that corresponds to the exception parameter and handles exceptions.
---*/

class ZeroDivisor extends Exception {}

function divide(a: int, b: int): int throws {
    if (b == 0) throw new ZeroDivisor()
    return a / b
}

const MaxInt = 2147483647

function process(a: int, b: int): int {
    try {
        let res = divide(a, b)
        return res
    }
    catch (d: ZeroDivisor) {
        return MaxInt
    }
    catch (e) {
        return 0
    }
}

function main() {
    assert process(4, 2) == 2
    assert process(4, 0) == MaxInt
}
