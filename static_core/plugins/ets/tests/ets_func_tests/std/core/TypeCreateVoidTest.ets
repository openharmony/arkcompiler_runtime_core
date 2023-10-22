/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function foo(): void {}

function test(a: Object): void {
	assert(a instanceof Void);
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
			.addBaseType(Type.of(new C()) as ClassType)
			.addMethod(
				new MethodCreator("foo")
					.addResult(VoidType.REF)
					.addBody(new CallableBodyFunction(release))
			)
			.addMethod(
				new MethodCreator("bar")
					.addResult(VoidType.REF)
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
	test(c1.bar())

	// TODO(kprokopenko): as this function is called multiple times,
	// same class creation will lead to mismatched lambda context...
	//assert called[0];
}
