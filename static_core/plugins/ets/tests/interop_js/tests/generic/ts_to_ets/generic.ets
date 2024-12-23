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
	tsString,
	LiteralClass,
	UnionClass,
	InterfaceClass,
	AbstractClass,
	GClass,
	literalClass,
	unionClass,
	interfaceClass,
	abstractClass,
	genericFunction,
	tupleDeclaredType,
	explicitlyDeclaredType,
} from 'dynamic_import';

const strContent = 'ets_test_string';
const numContent = 1;
const boolContent = true;

function checkLiteralClassGeneric(): boolean {
	const literalClass = new LiteralClass(strContent);

	return strContent == literalClass.get();
}

function checkUnionClassGeneric(): boolean {
	const unionClass = new UnionClass(strContent);

	return strContent == unionClass.get();
}

function checkInterfaceClassGeneric(): boolean {
	const interfaceClass = new InterfaceClass(strContent);

	return strContent == interfaceClass.get();
}

function checkAbstractClassGeneric(): boolean {
	const abstractClass = new AbstractClass(strContent);

	return strContent == abstractClass.get();
}

function checkGenericClassGeneric(): boolean {
	const gClass = new GClass(strContent);

	return strContent == gClass.get();
}

function checkGenericLiteralClassObjectFromTs(): boolean {
	return tsString === literalClass.get();
}

function checkGenericUnionClassObjectFromTs(): boolean {
	return tsString === unionClass.get();
}

function checkGenericInterfaceClassObjectFromTs(): boolean {
	return tsString === interfaceClass.get();
}

function checkGenericAbstractClassObjectFromTs(): boolean {
	return tsString === abstractClass.get();
}

function checkGenericFunctionTypeAnyString(): boolean {
	return strContent == genericFunction(strContent);
}

function checkGenericFunctionTypeAnyInt(): boolean {
	return numContent == genericFunction(numContent);
}

function checkGenericFunctionTypeAnyBool(): boolean {
	return (boolContent as boolean) == (genericFunction(boolContent) as boolean);
}

function checkGenericFunctionTupleType() {
	const arr: [int, string] = [numContent, strContent];
	const result = tupleDeclaredType<[int, string]>(arr);
	return arr[0] == result[0] && arr[1] == result[1];
}

function checkGenericExplicitlyDeclaredType(): boolean {
	return tsString === explicitlyDeclaredType();
}
