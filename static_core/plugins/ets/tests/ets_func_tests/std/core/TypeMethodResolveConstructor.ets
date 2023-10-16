let called = -1

interface A {}
class B implements A {}
class D implements A {}

const AType = Type.resolve('LA;') as InterfaceType
const BType = Type.of(new B()) as ClassType
const CType = Type.of(new C()) as ClassType

class C {
    constructor() {
        called = 0
    }

    constructor(a: int) {
        called = 1
    }

    constructor(a: A) {
        called = 11
    }
    constructor(a: B) {
        called = 12
    }

    constructor(a: int, b: int) {
        called = 2
    }
}

function test(args: NullishType[], exp: int): int {
    called = -1
    CType.make(args)
    if (called != exp) {
        console.println("FAILED: " + exp + " got " + called)
        console.println(args)
        return 1
    } else {
        console.log("ok " + exp)
    }
    return 0
}

function mk(num: int, fn: (idx: int) => NullishType): NullishType[] {
    const arr = new NullishType[num]
    for (let i = 0; i < num; i++) {
        arr[i] = fn(i)
    }
    return arr;
}

function from(x: NullishType): NullishType[] {
    const ret = new NullishType[1]
    ret[0] = x
    return ret
}

function main(): int {
    let f = 0
    f += test(new Object[0], 0)
    f += test(mk(2, (idx: int): NullishType => { return idx as Int }), 2)
    f += test(from(new B()), 12)
    f += test(from(new D()), 11)

    let threw = false
    try {
        CType.make(from(new C()))
    } catch (e) {
        threw = true
    }
    if (!threw) {
        f += 1
        console.println("exception not thrown")
    }
    return f
}
