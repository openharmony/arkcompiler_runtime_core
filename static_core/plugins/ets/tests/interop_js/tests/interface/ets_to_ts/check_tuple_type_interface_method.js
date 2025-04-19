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

const { num, string, TupleTypeMethodClass, createInterfaceClassTupleTypeMethod } = require('interface_method.test.abc');

const arr = [num, string];

function checkTupleTypeInterfaceMethod() {
	const IClass = new TupleTypeMethodClass();

	const result = IClass.get(arr);
	ASSERT_TRUE(Array.isArray(result) && result[0] === num && result[1] === string);
}

function checkCreateTupleTypeClassFromEts() {
	const IClass = createInterfaceClassTupleTypeMethod();

	const result = IClass.get(arr);
	ASSERT_TRUE(Array.isArray(result) && result[0] === num && result[1] === string);
}

checkTupleTypeInterfaceMethod();
checkCreateTupleTypeClassFromEts();
