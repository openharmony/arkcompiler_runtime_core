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
const { init, validationXGCResult, triggerXGC, JsTestClass } = require('./mark_test_xgc_utils.js');
let g_etsVm = init('mark_test_xgc_module', 'mark_test_xgc.abc');
let StsTestClass = g_etsVm.getClass('Lxgc_test_ets/TestStsClass;');

function myObjectTest() {
    let obj1 = new JsTestClass();
    let obj2 = new StsTestClass();
    let obj3 = new JsTestClass();
    let obj4 = new StsTestClass();
    let obj5 = new JsTestClass();
    let obj6 = new StsTestClass();
    obj1.obj = obj2;
    obj2.obj = obj3;
    obj3.obj = obj4;
    obj4.obj = obj5;
    obj5.obj = obj6;
    obj6.obj = obj1;
}
myObjectTest();
triggerXGC();
print('mark_test_xgc executes successful');
