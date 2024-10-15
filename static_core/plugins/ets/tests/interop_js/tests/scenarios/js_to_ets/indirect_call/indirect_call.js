/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
'use strict';

function indirectCallTypeAny(arg) {
	return arg;
}

function indirectCallTypeLiteral(arg) {
	const SUFFIX = 'bar';
	return arg + SUFFIX;
}

function indirectCallTypeExtraSet(tuple) {
	let [str, num] = tuple;
	const SUFFIX = 'bar';

	return [str + SUFFIX, num + 1];
}

class UserClass {
	method(arg) {
		return arg;
	}
}

/* Following TS code is transpiled to JS
interface UserInterface {
	interfaceMethod(arg: number): number;
}

class UserInterfaceImpl implements UserInterface {
	interfaceMethod(arg: number): number {
		return arg;
	}
}

function interfaceFunc(): UserInterface {
	let impl: UserInterface = new UserInterfaceImpl();
	return impl;
}
*/
class UserInterfaceImpl {
	interfaceMethod(arg) {
		return arg;
	}
}

function interfaceFunc() {
	let impl = new UserInterfaceImpl();
	return impl;
}

function indirectCallUnion(arg) {
	return arg;
}

function indirectCallTypeByRefArray(arg) {
	return [...arg];
}

function indirectCallTypeByRefTuple(arg) {
	return [...arg];
}

function indirectCallTypeByRefMap(arg) {
	const KEY = 'key2';
	const VALUE = 2;

	let result = new Map(arg);
	result.set(KEY, VALUE);

	return result;
}

function indirectCallTypeByValueNumber(arg) {
	return +arg + +arg;
}

function indirectCallTypeByValueString(arg) {
	return ''.concat(arg, arg);
}

function indirectCallTypeByValueBoolean(arg) {
	return !Boolean(arg);
}

exports.indirectCallTypeAny = indirectCallTypeAny;
exports.indirectCallTypeLiteral = indirectCallTypeLiteral;
exports.indirectCallTypeExtraSet = indirectCallTypeExtraSet;
exports.UserClass = UserClass;
exports.interfaceFunc = interfaceFunc;
exports.indirectCallUnion = indirectCallUnion;
exports.indirectCallTypeByRefArray = indirectCallTypeByRefArray;
exports.indirectCallTypeByRefTuple = indirectCallTypeByRefTuple;
exports.indirectCallTypeByRefMap = indirectCallTypeByRefMap;
exports.indirectCallTypeByValueNumber = indirectCallTypeByValueNumber;
exports.indirectCallTypeByValueString = indirectCallTypeByValueString;
exports.indirectCallTypeByValueBoolean = indirectCallTypeByValueBoolean;
