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
import * as test from "test";

loadLibrary("ani_compare");

function testCallbackManager() {
    let manager = new test.CallbackManager();

    let callback_1 = () => {
        console.log("Callback 1 executed");
        return "Callback 1 result";
    };
    let callback_2 = () => {
        console.log("Callback 2 executed");
        return "Callback 2 result";
    };
    let callback_3 = () => {
        console.log("Callback 3 executed");
        return "Callback 3 result";
    };

    let str_1 = callback_1();
    let str_2 = callback_2();
    let str_3 = callback_3();

    assertEQ(manager.addCallback(callback_1), true);
    assertEQ(manager.addCallback(callback_2), true);
    assertEQ(manager.addCallback(callback_1), false);

    let arr_a = manager.invokeCallbacks();
    assertEQ(arr_a.length, 2);
    assertEQ(arr_a[0], str_1);
    assertEQ(arr_a[1], str_2);

    assertEQ(manager.removeCallback(callback_1), true);
    assertEQ(manager.removeCallback(callback_3), false);

    let arr_b = manager.invokeCallbacks();
    assertEQ(arr_b.length, 1);
    assertEQ(arr_b[0], str_2);
}

function main() {
    let suite = new ArkTestsuite("CallbackManager Test Suite");

    suite.addTest("CallbackManager Test", testCallbackManager);

    exit(suite.run());
}
