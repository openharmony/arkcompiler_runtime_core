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
const { etsVm, getTestModule } = require('operator_new.test.abc');

const etsMod = getTestModule('operator_new_ets_to_js_test');
const GCJSRuntimeCleanup = etsMod.getFunction('GCJSRuntimeCleanup');

const twoArgClass = etsMod.getClass('TwoArgClass');

{
    let ret = new twoArgClass('TestName', 'TestCity');
    ASSERT_EQ(ret.name === 'TestName' && ret.city === 'TestCity', true);
}
