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

const { etsVm, getTestModule } = require('objects_passing/objects_passing_sts.test.abc');

const etsMod = getTestModule('objects_passing_ets_test');
const GCJSRuntimeCleanup = etsMod.getFunction('GCJSRuntimeCleanup');
const getValueObject = etsMod.getFunction('getObjectName');
const getOuterObject = etsMod.getFunction('getOuterObject');

const testObject = {
	name: 'TestName',
};

const testInnerObject = { id: 123 };

const testOuterObject = {
	id: 456,
	testInnerObject: testInnerObject,
};

{
	let ret = getValueObject(testObject);
	print(ret);
	ASSERT_EQ(ret, 'TestName');
}

{
	let ret = getOuterObject(testOuterObject);
	print(ret);
	ASSERT_EQ(ret, 456);
}
