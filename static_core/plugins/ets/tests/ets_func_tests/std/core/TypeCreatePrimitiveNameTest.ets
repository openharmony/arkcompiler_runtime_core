/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

interface I {
	i(x: int): int { return 0 }
}

function foo(self: Object, x: int): int {
	assertEQ( (Type.of(self) as ClassType).getName(), "i32")
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
				.addParameter(new ParameterCreator(i32))
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
	assertEQ(i.i(19), 30)
}
