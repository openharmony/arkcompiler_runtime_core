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

const { etsVm, getTestModule } = require('escompat.test.js');

const etsMod = getTestModule('escompat_test');
const StringifyJSValue = etsMod.getFunction('JSON_stringify_jsv');
const StringifyObject = etsMod.getFunction('JSON_stringify_obj');
const StringifyField = etsMod.getFunction('JSON_stringify_field');
const FooClass = etsMod.getClass('FooClass');

class B {
	z = 123;
}
class A {
	x = '123';
	y = new B();
}

class C {
	toJSON() {
		throw 123;
	}
}

function tmp() {}

{
	// Test ETS stringify
	let err = null;
	try {
		StringifyJSValue(tmp); // JSON.stringify(function) returns undefined (not a string!). We throw exception
	} catch (e) {
		err = e;
	}
	ASSERT_EQ(String(err.message), 'String expected');

	let jsv = StringifyJSValue(new A());
	let obj = StringifyObject(new A());
	let jsvsield = StringifyField(new A());
	ASSERT_EQ(jsv, '{"x":"123","y":{"z":123}}');
	ASSERT_EQ(obj, '{"x":"123","y":{"z":123}}');

	ASSERT_EQ(jsvsield, '{"x":{"x":"123","y":{"z":123}}}');

	try {
		let etsCodeFromJson = StringifyJSValue(new C());
		throw 124;
	} catch (e) {
		ASSERT_EQ(e, 123);
	}
}
