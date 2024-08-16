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
const { etsVm, getTestModule } = require('indirect_call.test.js');

const etsMod = getTestModule('indirect_call_test_type_ref');
const GCJSRuntimeCleanup = etsMod.getFunction('GCJSRuntimeCleanup');
const function_type_ref_tuple = etsMod.getFunction('function_type_ref_tuple');

function test_function_type_ref_tuple_apply() {
	const ARG = [42, '73'];
	let result = function_type_ref_tuple.apply(null, [ARG]);

	ASSERT_EQ(JSON.stringify(result), JSON.stringify(ARG));
}

test_function_type_ref_tuple_apply();

GCJSRuntimeCleanup();
