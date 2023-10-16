function foo(): void {}

function test(a: Object): void {
	assert(a instanceof void);
}

class C {
	foo(): void { throw new Error("not overriden")  }
	bar(): void { throw new Error("not overriden")  }
}

function main(): void throws {
	test(foo());

	const called = new boolean[1];

	const release: (a: Object) => void = (a: Object): void => {
		called[0] = true;
	}

	const creator =
		new ClassTypeCreator("C1")
			.addBase(Type.of(new C()) as ClassType)
			.addMethod(
				new MethodCreator("foo")
					//.addResult(VoidType.REF)
					.addResult(Type.of(Void))
					.addBody(new CallableBodyFunction(release))
			)
			.addMethod(
				new MethodCreator("bar")
					//.addResult(VoidType.REF)
					.addResult(Type.of(Void))
					.addBody(new CallableBodyDefault())
			)
			.addMethod(
				new MethodCreator("constructor")
					.addConstructor()
					.addResult(VoidType.REF)
					.addBody(new CallableBodyDefault())
			)
			;

	const C1 = creator.create()
	const c1 = C1.make() as C

	test(c1.foo())
	//test(c1.bar())

	// TODO(kprokopenko): as this function is called multiple times,
	// same class creation will lead to mismatched lambda context...
	//assert called[0];
}
