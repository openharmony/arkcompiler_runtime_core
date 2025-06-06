/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

const { etsVm, getTestModule } = require('../escompat.test.abc');

const etsMod = getTestModule('escompat_test');
const GCJSRuntimeCleanup = etsMod.getFunction('GCJSRuntimeCleanup');
const FooClass = etsMod.getClass('FooClass');
const CreateEtsSample = etsMod.getFunction('Array_CreateEtsSample');
const TestJSFind = etsMod.getFunction('Array_TestJSFind');

{
	// Test JS Array<FooClass>
	TestJSFind(new Array(new FooClass('zero'), new FooClass('one')));
}

{
	// Test ETS Array<Object>
	let arr = CreateEtsSample();
	function fnTrue(v) {
		return true;
	}
	function fnFalse(v) {
		return false;
	}

	// reference check for how this is supposed to behave:
	ASSERT_EQ([1, 2, 3].find(fnTrue), 1);
	ASSERT_TRUE([1, 2, 3].find(fnFalse) === null); // null or undefined

	// actual test code:
	let found = arr.find(fnTrue);
	ASSERT_EQ(found, 123);

	let foundNull = arr.find(fnFalse);
	console.log(foundNull);
	ASSERT_TRUE(foundNull === null);
}

GCJSRuntimeCleanup();
