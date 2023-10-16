class C {
    getIsEvenPlusX(x: int): boolean {
        return (this.a + x) % 2 == 0
    }

    static sum(a: int, b: int): long {
        return a as long + b
    }

    a: int = 0
}

function testSum(sum: Method): void {
    const args = new Object[2]
    args[0] = Int.MAX_VALUE
    args[1] = 30
    const res = sum.invoke(null, args) as Long
    assert res == (Int.MAX_VALUE as long + 30)
}

function testGetIsEvenPlusX(getIsEvenPlusX: Method): void {
    const c = new C()
    c.a = 15
    const args = new Object[1]

    args[0] = 13
    assert getIsEvenPlusX.invoke(c, args) as Boolean == true

    c.a = 14
    assert getIsEvenPlusX.invoke(c, args) as Boolean == false

    args[0] = 30
    assert getIsEvenPlusX.invoke(c, args) as Boolean == true
}

function main(): void {
    const c = new C()
    const cType = Type.of(c) as ClassType
    const meths = cType.getMethodsNum()
    let got = 0
    for (let i = 0; i < meths; i++) {
        const m = cType.getMethod(i)
        if (m.getName() == "sum") {
            testSum(m)
            got++
        }
        if (m.getName() == "getIsEvenPlusX") {
            testGetIsEvenPlusX(m)
            got++
        }
    }
    assert got == 2
}
