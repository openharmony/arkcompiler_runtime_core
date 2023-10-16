class Foo {
    fld: int = 11

    public bar(): int {
        return this.fld
    }
}

function fooType(): ClassType {
    return Type.of(new Foo()) as ClassType // Type.resolve("LETSGLOBAL/Foo;") as ClassType
}

function main(): void throws {
    const fooClass = fooType()
    const creator = new ClassTypeCreator("MyClass").addBase(fooClass)
    assert fooClass.getConstructorsNum() == 1
    const ctor = fooClass.getConstructor(0)
    creator.addMethod(
        new MethodCreator("constructor")
            .addConstructor()
            .addBody(new CallableBodyMethod(ctor))
    )
    let body: (x: Foo) => int = (x: Foo): int => { return x.fld + 30 }
    creator.addMethod(
        new MethodCreator("bar")
        .addResult(IntType.VAL)
        .addBody(new CallableBodyFunction(body as Object))
    )

    const cls = creator.create()

    const f = cls.make() as Foo
    assert f.bar() == 41
}
