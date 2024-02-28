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
const { getTestModule } = require('scenarios.test.js');

const etsMod = getTestModule('scenarios_test');
const functionIntersectionTypePrimitiveEts = etsMod.getFunction('functionIntersectionTypePrimitive');
const functionIntersectionTypeObjectEts = etsMod.getFunction('functionIntersectionTypeObject');

{
	const VALUE1 = '1';

	let ret = functionIntersectionTypePrimitiveEts(1);
	ASSERT_EQ(ret, 1);

	let res = functionIntersectionTypeObjectEts();
	ASSERT_EQ(res.a, 1);
	ASSERT_EQ(res.b, VALUE1);
	ASSERT_TRUE(typeof res.a === 'number');
	ASSERT_TRUE(typeof res.b === 'string');
}
