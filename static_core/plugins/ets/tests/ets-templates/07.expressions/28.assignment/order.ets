/*---
desc: >-
    All assignment operators are syntactically right-associative (they group right-to-left).
    Thus, a=b=c means a=(b=c), which assigns the value of c to b and then assigns the value of b to a.
params: with and without braces
---*/

let result: String = ""

function getIdx(i: int): int {
    result += i
    return i
}

function main(): int {

    // without braces
    result = ""
    let x: int[] = new int[3]
    let t: int = x[getIdx(0)] = x[getIdx(1)] = x[getIdx(2)] = 3
    if (result != "012" || t != 3) {
        return 1
    }

    // with braces
    result = ""
    let y: int[] = new int[3]
    let q: int = (y[getIdx(0)] = (y[getIdx(1)] = (y[getIdx(2)] = 3)))
    if (result != "012" || q != 3) {
        return 1
    }

    return 0
}
