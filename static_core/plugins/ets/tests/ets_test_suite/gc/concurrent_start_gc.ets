/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

function alloc(): void {
    for (let i = 0; i < 100; ++i) {
        new Object();
    }
}

let flag: boolean = false;

function main(): int {
    alloc();
    let gc: long = -100;
    try {
        gc = GC.startGC(GC.Cause.THRESHOLD, (): void => {
            flag = true;
        });
    }
    catch (e) {
        assertTrue(false, "startGC must not throw exceptions in this case");
    }
    assertEQ(gc, 0, "GC with callback must be run in place");
    assertTrue(flag, "Callback hasn't been called");
    return 0;
}
