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
const { etsVm, getTestModule } = require('scenarios.test.js');

const etsMod = getTestModule('interopStandaloneFnTest');
const functionArgTypeUnionEts = etsMod.getFunction('functionArgTypeUnionEts');

{
	let testString = 'test';
	let testNumber = 1;
	let resString = functionArgTypeUnionEts(testString);
	// calling similar JS method also gets type string not string
	ASSERT_TRUE(typeof resString === 'string', "type 'string' expected");
	ASSERT_EQ(resString, 'test');
	let resNumber = functionArgTypeUnionEts(testNumber);
	// calling similar JS method also gets type string not string
	ASSERT_TRUE(typeof resNumber === 'number', "type 'number' expected");
	ASSERT_EQ(resNumber, 1);
}
