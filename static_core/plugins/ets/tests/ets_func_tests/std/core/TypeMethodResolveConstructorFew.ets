let called = -1

interface I {}
interface J {}
class A implements I {}
class B implements J {}
class C implements I, J {}

const TestType = Type.of(new Test()) as ClassType

class Test {
    constructor() {}

    constructor(x: I) {
        called = 1
    }

    constructor(x: J) {
        called = 2
    }
}

function test(args: NullishType[], exp: int): int {
    called = -1
    try {
        TestType.make(args)
    } catch (e) {
        console.println(e.toString())
        called = -1
    }
    if (called != exp) {
        console.println("FAILED: " + exp + " got " + called)
        console.println(args)
        return 1
    } else {
        console.log("ok " + exp)
    }
    return 0
}

function from(x: NullishType): NullishType[] {
    const ret = new NullishType[1]
    ret[0] = x
    return ret
}

function main(): int {
    let f = 0
    f += test(from(new A()), 1)
    f += test(from(new B()), 2)

    let threw = false
    try {
        TestType.make(from(new C()))
    } catch (e) {
        threw = true
    }
    if (!threw) {
        f += 1
        console.println("exception not thrown")
    }
    return f
}
