/*
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
package esobject_to_promise_test;

async function checkToPromise() {
    let module = ESObject.load("../../plugins/ets/tests/interop_js/tests/test_esobject_to_promise/esobject_to_promise_test");
    let sleepRetNumber = module.getProperty("sleepRetNumber");

    let p = sleepRetNumber.invoke(ESObject.wrapNumber(5000)).toPromise();
    let res = await p;

    assertTrue(res.toNumber() === 0xcafe);
}
