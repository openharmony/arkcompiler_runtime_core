/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import {
	tsNumber,
	tsString,
	AnyTypeMethodClass,
	createInterfaceClassAnyTypeMethod,
	UnionTypeMethodClass,
	createInterfaceClassUnionTypeMethod,
	unionTypeMethodInstanceClass,
	subsetByRefInterface,
	SubsetByValueClass,
	createSubsetByValueClassFromTs,
	subsetByValueInstanceClass,
	TupleTypeMethodClass,
	createInterfaceClassTupleTypeMethodFromTs,
	tupleInstanceClass,
	WithOptionalMethodClass,
	WithoutOptionalMethodClass,
	createClassWithOptionalMethod,
	createClassWithoutOptionalMethod,
	withoutOptionalMethodInstanceClass,
	withOptionalMethodInstanceClass,
	optionalArg,
	optionalArgArray,
} from 'dynamic_import'

const str = 'string'
const bool = true
const num = 1
const arr = []
const obj: Object = {}
const arrTuple: [int, string] = [num, str]

class ObjMethod {
	get(): int {
		return num
	}
}

function checkAnyTypeInterfaceClassString(): boolean {
	const anyTypeMethodClass = new AnyTypeMethodClass()

	return (str as string) == (anyTypeMethodClass.get(str) as string)
}

function checkAnyTypeInterfaceClassInt(): boolean {
	const anyTypeMethodClass = new AnyTypeMethodClass()

	return (num as int) == (anyTypeMethodClass.get(num) as int)
}

function checkAnyTypeInterfaceClassBool(): boolean {
	const anyTypeMethodClass = new AnyTypeMethodClass()

	return (bool as boolean) == (anyTypeMethodClass.get(bool) as boolean)
}

function checkAnyTypeInterfaceClassArray(): boolean {
	const anyTypeMethodClass = new AnyTypeMethodClass()

	return (anyTypeMethodClass.get(arr) as Array<int>) instanceof Array
}

function checkAnyTypeInterfaceClassObject(): boolean {
	const anyTypeMethodClass = new AnyTypeMethodClass()

	return 'object' == (typeof anyTypeMethodClass.get(obj) as Object)
}

function checkCreateInterfaceClassAnyTypeMethodString(): boolean {
	const anyTypeMethodClass = createInterfaceClassAnyTypeMethod()

	return (str as string) == (anyTypeMethodClass.get(str) as string)
}

function checkCreateInterfaceClassAnyTypeMethodInt(): boolean {
	const anyTypeMethodClass = createInterfaceClassAnyTypeMethod()

	return (num as int) == (anyTypeMethodClass.get(num) as int)
}

function checkCreateInterfaceClassAnyTypeMethodBool(): boolean {
	const anyTypeMethodClass = createInterfaceClassAnyTypeMethod()

	return (bool as boolean) == (anyTypeMethodClass.get(bool) as boolean)
}

function checkCreateInterfaceClassAnyTypeMethodArray(): boolean {
	const anyTypeMethodClass = createInterfaceClassAnyTypeMethod()

	return (anyTypeMethodClass.get(arr) as Array<int>) instanceof Array
}

function checkCreateInterfaceClassAnyTypeMethodObject(): boolean {
	const anyTypeMethodClass = createInterfaceClassAnyTypeMethod()

	return 'object' == (typeof anyTypeMethodClass.get(obj) as Object)
}

function checkUnionTypeInterfaceClassString(): boolean {
	const unionTypeMethodClass = new UnionTypeMethodClass()

	return (str as string) == (unionTypeMethodClass.get(str) as string)
}

function checkUnionTypeInterfaceClassInt(): boolean {
	const unionTypeMethodClass = new UnionTypeMethodClass()

	return (num as int) == (unionTypeMethodClass.get(num) as int)
}

function checkCreateInterfaceClassUnionTypeMethodString(): boolean {
	const unionTypeMethodClass = createInterfaceClassUnionTypeMethod()

	return (str as string) == (unionTypeMethodClass.get(str) as string)
}

function checkCreateInterfaceClassUnionTypeMethodInt(): boolean {
	const unionTypeMethodClass = createInterfaceClassUnionTypeMethod()

	return (num as int) == (unionTypeMethodClass.get(num) as int)
}

function checkInstanceInterfaceClassUnionTypeMethodString(): boolean {
	return (str as string) == (unionTypeMethodInstanceClass.get(str) as string)
}

function checkInstanceInterfaceClassUnionTypeMethodInt(): boolean {
	return (num as int) == (unionTypeMethodInstanceClass.get(num) as int)
}

function checkSubsetByRefInterface(): boolean {
	return (num as int) == (subsetByRefInterface(new ObjMethod()) as int)
}

function checkSubsetByRefInterfaceError(): boolean {
	try {
		subsetByRefInterface(obj)

		return false
	} catch (e) {
		return true
	}
}

function checkSubsetByValueClass(): boolean {
	const subsetByValueClass = new SubsetByValueClass()
	const userClass = subsetByValueClass.get()

	return (tsNumber as int) == (userClass.value as int)
}

function checkCreateSubsetByValueClassFromTs(): boolean {
	const subsetByValueClass = createSubsetByValueClassFromTs()
	const userClass = subsetByValueClass.get()

	return (tsNumber as int) == (userClass.value as int)
}

function checkInstanceSubsetByValueClassFromTs(): boolean {
	const userClass = subsetByValueInstanceClass.get()

	return (tsNumber as int) == (userClass.value as int)
}

function checkTupleTypeClass(): boolean {
	const tupleClass = new TupleTypeMethodClass()

	const result = tupleClass.get(arrTuple)
	return (result as Array<[int, string]>) instanceof Array && (result[0] as int) == (num as int) && (result[1] as string) == (str as string)
}

function checkCreateTupleClassFromTs(): boolean {
	const tupleClass = createInterfaceClassTupleTypeMethodFromTs()

	const result = tupleClass.get(arrTuple)
	return (result as Array<[int, string]>) instanceof Array && (result[0] as int) == (num as int) && (result[1] as string) == (str as string)
}

function checkInstanceTupleClassFromTs(): boolean {
	const result = tupleInstanceClass.get(arrTuple)
	return (result as Array<[int, string]>) instanceof Array && (result[0] as int) == (num as int) && (result[1] as string) == (str as string)
}

function checkWithOptionalMethodClass(): boolean {
	const optionalClass = new WithOptionalMethodClass()

	return (optionalClass.getNum() as int) == (tsNumber as int) && (optionalClass.getStr() as string) == (tsString as string)
}

function checkWithoutOptionalMethodClass(): boolean {
	const optionalClass = new WithoutOptionalMethodClass()

	return (optionalClass.getStr() as string) == (tsString as string)
}

function checkCreateClassWithOptionalMethod() {
	const optionalClass = createClassWithOptionalMethod()

	return (optionalClass.getNum() as int) == (tsNumber as int) && (optionalClass.getStr() as string) == (tsString as string)
}

function checkCreateClassWithoutOptionalMethod() {
	const optionalClass = createClassWithoutOptionalMethod()

	return (optionalClass.getStr() as string) == (tsString as string)
}

function checkWithOptionalMethodInstanceClass(): boolean {
	return (withOptionalMethodInstanceClass.getNum() as int) == (tsNumber as int) && (withOptionalMethodInstanceClass.getStr() as string) == (tsString as string)
}

function checkWithoutOptionalMethodInstanceClass() {
	return (withoutOptionalMethodInstanceClass.getStr() as string) == (tsString as string)
}

function checkOptionalArgWithAllArgs(): boolean {
	const result = optionalArg(withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass)

	return !!result.with && !!result.without
}

function checkOptionalArgWithOneArgs(): boolean {
	const result = optionalArg(withOptionalMethodInstanceClass)

	return !!result.with
}

// NOTE (issues 17772) fix spread operator and
// uncomment function in checkSpreadOperatorArgWithAllArgs

// function checkSpreadOperatorArgWithAllArgs(): boolean {
//     const arr: FixedArray<WithoutOptionalMethodClass | withoutOptionalMethodInstanceClass> = [withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass];
//     const result = optionalArgArray(...arr);
//     return !!result.with && !!result.without;
// }

// function checkSpreadOperatorArgWithOneArgs(): boolean {
//     const arr: FixedArray<WithoutOptionalMethodClass | WithoutOptionalMethodClass> = [withOptionalMethodInstanceClass];
//     const result = optionalArgArray(...arr);
//     return !!result.with;
// }

function checkWithoutSpreadOperatorArgWithAllArgs(): boolean {
	const result = optionalArgArray(withOptionalMethodInstanceClass, withoutOptionalMethodInstanceClass)
	return !!result.with && !!result.without
}

function checkWithoutSpreadOperatorArgWithOneArgs(): boolean {
	const result = optionalArgArray(withOptionalMethodInstanceClass)
	return !!result.with
}
