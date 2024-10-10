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

const { num, string, UnionTypeMethodClass, createInterfaceClassUnionTypeMethod } = require('interface_method.test.js');

function checkUnionTypeMethodClassInterfaceInt() {
	const IClass = new UnionTypeMethodClass();

	ASSERT_TRUE(IClass.get(num) === num);
}

function checkUnionTypeMethodClassInterfaceString() {
	const IClass = new UnionTypeMethodClass();

	ASSERT_TRUE(IClass.get(string) === string);
}

function checkUnionInterfaceObjectFromEtsInt() {
	const IClass = new createInterfaceClassUnionTypeMethod();

	ASSERT_TRUE(IClass.get(num) === num);
}

function checkUnionInterfaceObjectFromEtsString() {
	const IClass = new createInterfaceClassUnionTypeMethod();

	ASSERT_TRUE(IClass.get(string) === string);
}

function checkUnionValueTypeMethodClassInterfaceInt() {
	const IClass = new UnionTypeMethodClass();

	ASSERT_TRUE(IClass.get(num) === num);
}

function checkUnionValueTypeMethodClassInterfaceString() {
	const IClass = new UnionTypeMethodClass();

	ASSERT_TRUE(IClass.get(string) === string);
}

function checkUnionValueInterfaceObjectFromEtsInt() {
	const IClass = new createInterfaceClassUnionTypeMethod();

	ASSERT_TRUE(IClass.get(num) === num);
}

function checkUnionValueInterfaceObjectFromEtsString() {
	const IClass = new createInterfaceClassUnionTypeMethod();

	ASSERT_TRUE(IClass.get(string) === string);
}

checkUnionTypeMethodClassInterfaceInt();
checkUnionTypeMethodClassInterfaceString();
checkUnionInterfaceObjectFromEtsInt();
checkUnionInterfaceObjectFromEtsString();
checkUnionValueTypeMethodClassInterfaceInt();
checkUnionValueTypeMethodClassInterfaceString();
checkUnionValueInterfaceObjectFromEtsInt();
checkUnionValueInterfaceObjectFromEtsString();
