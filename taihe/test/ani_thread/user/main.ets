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
import {BusinessError} from "@ohos.base";
import * as threadTest from "thread_test";

loadLibrary("ani_thread");

function main() {
    threadTest.invokeFromOtherThreadAfter(1.0, () => {
        console.log("hello world");
    })
    // sleep 2 seconds to wait for the callback
    let start = Date.now();
    while (Date.now() - start < 2000) {
        // do nothing
    }
}
