class C {
	c(a1: int, a2: long, c: boolean): long {
		return 0
	}
	m(s: string): string {
		return ""
	}
}


function erasedM(args: Object[]): Object {
	return (args[1] as string) + "123"
}

function erased(args: Object[]): Object {
	const a1: int = args[1] as Int
	const a2: long = args[2] as Long
	const c: boolean = args[3] as Boolean
	if (c) {
		const r = a1 as long
		return r as Long
	} else {
		return a2 as Long
	}
}

function main(): void throws {
	const c = Type.of(new C()) as ClassType
	const creator = new ClassTypeCreator("C1").addBase(c)
	creator
		.addMethod(
			new MethodCreator("c")
				.addParameter(IntType.VAL)
				.addParameter(LongType.VAL)
				.addParameter(BooleanType.VAL)
				.addResult(LongType.VAL)
				.addBody(new CallableBodyErasedFunction(erased))
		)
		.addMethod(
			new MethodCreator("m")
				.addParameter(StringType.VAL)
				.addResult(StringType.VAL)
				.addBody(new CallableBodyErasedFunction(erasedM))
		)
		.addMethod(
			new MethodCreator("constructor")
				.addConstructor()
				.addBody(new CallableBodyDefault())
		)
	const ty = creator.create()
	const i = ty.make() as C
	assert(i.c(10, 11, true) == 10)
	assert(i.c(10, 11, false) == 11)
	assert(i.m("abc") == "abc123")
}
