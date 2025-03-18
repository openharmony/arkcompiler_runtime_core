/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

const etsVm = globalThis.gtest.etsVm;
const string = 'string';
const SetterAnyType = etsVm.getClass('Lsetter/test/SetterAnyType;');
const createSetterAnyTypeClass = etsVm.getFunction('Lsetter/test/ETSGLOBAL;', 'create_setter_any_type_class');


function checkSetterAnyType() {
	const setterAnyType = new SetterAnyType();
	setterAnyType.value = string;

	ASSERT_TRUE(string === setterAnyType.value);
}

function checkSetterAnyTypeClassFromEts() {
	const setterAnyType = createSetterAnyTypeClass();
	setterAnyType.value = string;

	ASSERT_TRUE(string === setterAnyType.value);
}

checkSetterAnyType();
checkSetterAnyTypeClassFromEts();
