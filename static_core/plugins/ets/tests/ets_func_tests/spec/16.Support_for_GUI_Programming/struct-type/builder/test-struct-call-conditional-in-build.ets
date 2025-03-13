/*
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

/*---
desc: Chapter 16. Support for GUI Programming
name: test-struct-call-conditional-in-build.ets
tags:
- compile-only

---*/


struct Test {
    flag: boolean = true;

    build() {
        if(flag) {
            this.callMethodOne();
        } else {
            this.callMethodTwo();
        }
    }
    @Builder
    callMethodOne() {
    }
    @Builder
    callMethodTwo() {
    }
}

function main(): int {
    return 0;
}
