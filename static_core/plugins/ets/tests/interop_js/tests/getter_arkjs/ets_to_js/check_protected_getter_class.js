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

import { PROTECTED_GETTER_CLASS_VALUE } from './constant';

const etsVm = globalThis.gtest.etsVm;
const ProtectedGetterOrigenClass = etsVm.getClass('Lgetter/test/ProtectedGetterOrigenClass;');
const createProtectedGetterOrigenClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_protected_getter_origen_class_from_ets');


function checkProtectedGetterClassalue() {
	const GClass = new ProtectedGetterOrigenClass();
	ASSERT_TRUE(GClass.alue === undefined);
}

function checkCreateProtectedGetterClassalueFromEts() {
	const GClass = createProtectedGetterOrigenClassFromEts();
	ASSERT_TRUE(GClass.alue === undefined);
}

function checkProtectedGetterClass() {
	const GClass = new ProtectedGetterOrigenClass();
	ASSERT_TRUE(GClass.value === PROTECTED_GETTER_CLASS_VALUE);
}

function checkCreateProtectedGetterClassromEts() {
	const GClass = createProtectedGetterOrigenClassFromEts();
	ASSERT_TRUE(GClass.value === PROTECTED_GETTER_CLASS_VALUE);
}

checkProtectedGetterClassalue();
checkCreateProtectedGetterClassalueFromEts();
checkProtectedGetterClass();
checkCreateProtectedGetterClassromEts();
