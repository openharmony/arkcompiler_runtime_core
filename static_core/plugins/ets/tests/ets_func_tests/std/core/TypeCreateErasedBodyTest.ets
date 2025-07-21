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

class C {
	c(a1: int, a2: long, c: boolean): long {
		return 0
	}
	m(s: string): string {
		return ""
	}

	dat: string = "DATA!"
}


function erasedM(recv: NullishType, args: FixedArray<NullishType>): NullishType {
	assertEQ( args.length, 1)
	return (args[0] as string) + "123" + (recv as C).dat
}

function erased(recv: NullishType, args: FixedArray<NullishType>): NullishType {
	assertEQ( args.length, 3)
	const a1: int = args[0] as Int
	const a2: long = args[1] as Long
	const c: boolean = args[2] as Boolean
	if (c) {
		const r = a1 as long
		return r as Long
	} else {
		return a2 as Long
	}
}

function main(): void throws {
	const c = Type.of(new C()) as ClassType
	const creator = new ClassTypeCreator("C1").addBaseType(c)
	creator
		.addMethod(
			new MethodCreator("c")
				.addParameter(new ParameterCreator(IntType.VAL))
				.addParameter(new ParameterCreator(LongType.VAL))
				.addParameter(new ParameterCreator(BooleanType.VAL))
				.addResult(LongType.VAL)
				.addBody(new CallableBodyErasedFunction(erased))
		)
		.addMethod(
			new MethodCreator("m")
				.addParameter(new ParameterCreator(StringType.REF))
				.addResult(StringType.REF)
				.addBody(new CallableBodyErasedFunction(erasedM))
		)
		.addMethod(
			new MethodCreator("constructor")
				.addConstructor()
				.addBody(new CallableBodyDefault())
		)
	const ty = creator.create()
	const i = ty.make() as C
	assertEQ(i.c(10, 11, true), 10)
	assertEQ(i.c(10, 11, false), 11)
	assertEQ(i.m("abc"), "abc123DATA!")
}
