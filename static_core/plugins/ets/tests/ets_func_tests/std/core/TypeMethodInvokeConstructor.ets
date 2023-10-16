class C {
    constructor(a: int) {
        this.a = a
    }

    a: int
}

function main(): void {
    const c = new C(1)
    const cType = Type.of(c) as ClassType
    assert cType.getConstructorsNum() == 1
    const m = cType.getConstructor(0)
    const args = new Object[1];
    args[0] = 10
    let res = m.invoke(null, args) as C
    assert res.a == 10

    args[0] = 30
    res = m.invoke(null, args) as C
    assert res.a == 30
}
