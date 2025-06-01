/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class TestClass {
    static sum(a0: int, a1: int): int {
        let global = JSRuntime.getGlobal();
        if (!global){
            console.log("No global");
        }

        try {
            let f1: JSValue = (x: JSValue) => { JSRuntime.getGlobal().value1 = 1;};
            let f2: JSValue = (x: JSValue) => { for (let i = 0; i < 1000000; ++i);  JSRuntime.getGlobal().value2 = 2;};
            JSRuntime.getPropertyJSValue(JSRuntime.getGlobal(), 'Promise').resolve().then(f1);
            JSRuntime.getPropertyJSValue(JSRuntime.getGlobal(), 'Promise').resolve().then(f2);
        }
        catch (e) {
            console.log("Function failed");
        }

        assertEQ(JSRuntime.getGlobal().value1  as int, 1);
        assertEQ(JSRuntime.getGlobal().value2 as int, 2);
        return a0 + a1;
    }
}
