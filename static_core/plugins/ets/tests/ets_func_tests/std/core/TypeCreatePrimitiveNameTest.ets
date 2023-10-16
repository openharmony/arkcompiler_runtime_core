interface I {
	i(x: int): int { return 0 }
}

function foo(self: Object, x: int): int {
	assert (Type.of(self) as ClassType).getName() == "i32"
	return x + 11
}

const foo1: (self: Object, x: int) => int = foo;

class C implements I {}

function main(): void throws {
	const CType = Type.of(new C()) as ClassType
	const i32 = Type.of(0)
	const creator = new ClassTypeCreator("i32").addInterface(CType.getInterface(0))
	creator
		.addMethod(
			new MethodCreator("i")
				.addParameter(i32)
				.addResult(i32)
				.addBody(new CallableBodyFunction(foo1 as Object))
		)
		.addMethod(
			new MethodCreator("constructor")
				.addConstructor()
				.addBody(new CallableBodyDefault())
		)
	const ty = creator.create()
	const i = ty.make() as I
	assert(i.i(19) == 30)
}
