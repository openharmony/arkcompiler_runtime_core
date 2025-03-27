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

const { num, string, LiteralValueMethodClass, createInterfaceLiteralValueClassFromEts } = require('interface_method.test.abc');

function checkLiteralTypeMethodClassInterfaceInt() {
	const IClass = new LiteralValueMethodClass();

	ASSERT_TRUE(IClass.get(num) === num);
}

function checkLiteralTypeMethodClassInterfaceString() {
	const IClass = new LiteralValueMethodClass();

	ASSERT_TRUE(IClass.get(string) === string);
}

function checkLiteralInterfaceObjectFromEtsInt() {
	const IClass = new createInterfaceLiteralValueClassFromEts();

	ASSERT_TRUE(IClass.get(num) === num);
}

function checkLiteralInterfaceObjectFromEtsString() {
	const IClass = new createInterfaceLiteralValueClassFromEts();

	ASSERT_TRUE(IClass.get(string) === string);
}

function checkLiteralValueTypeMethodClassInterfaceInt() {
	const IClass = new LiteralValueMethodClass();

	ASSERT_TRUE(IClass.get(num) === num);
}

function checkLiteralValueTypeMethodClassInterfaceString() {
	const IClass = new LiteralValueMethodClass();

	ASSERT_TRUE(IClass.get(string) === string);
}

function checkLiteralValueInterfaceObjectFromEtsInt() {
	const IClass = new createInterfaceLiteralValueClassFromEts();

	ASSERT_TRUE(IClass.get(num) === num);
}

function checkLiteralValueInterfaceObjectFromEtsString() {
	const IClass = new createInterfaceLiteralValueClassFromEts();

	ASSERT_TRUE(IClass.get(string) === string);
}

checkLiteralTypeMethodClassInterfaceInt();
checkLiteralTypeMethodClassInterfaceString();
checkLiteralInterfaceObjectFromEtsInt();
checkLiteralInterfaceObjectFromEtsString();
checkLiteralValueTypeMethodClassInterfaceInt();
checkLiteralValueTypeMethodClassInterfaceString();
checkLiteralValueInterfaceObjectFromEtsInt();
checkLiteralValueInterfaceObjectFromEtsString();
