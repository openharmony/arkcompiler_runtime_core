class AHAH {
    constructor() {}
    constructor(a: int) {}

    meth(): int {
        return this.hoho
    }

    // TODO(kprokopenko): add test with reading this field jsut after resolve
    static fld: int = 30
    hoho: int = 3
}

function main() {
    const iType = Type.resolve('LAHAH;') as ClassType
    const ctorsN = iType.getConstructorsNum()

    assert iType.getConstructorsNum() == 2
    iType.getConstructor(1).getName()

    let ok = false
    for (let i = 0; i < iType.getMethodsNum(); i++) {
        const m = iType.getMethod(i)
        if (m.getName() == "meth") {
            assert m.getType().getResultType() instanceof IntType
            ok = true
            break
        }
    }
    assert ok
    assert (iType.make() as AHAH).meth() == 3
}
