abstract class A {
    abstract foo(): int;
}

class C extends A {
    override foo(): int {
        return 30
    }
}

function main(): void {
    const a = Type.resolve('LA;') as ClassType
    let got = 0
    for (let i = 0; i < a.getMethodsNum(); i++) {
        const m = a.getMethod(i)
        if (m.getName() != "foo") {
            continue
        }
        got++
        assert m.invoke(new C(), new NullishType[0]) as Int == 30
    }
    assert got == 1
}
