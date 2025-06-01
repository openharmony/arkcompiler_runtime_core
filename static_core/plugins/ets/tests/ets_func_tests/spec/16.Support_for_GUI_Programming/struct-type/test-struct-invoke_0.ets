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
name: test-struct-invoke_0.ets
tags:

---*/
let globalVar: int = 0;

@Component
struct Test {
    build() {}
    static $_invoke(): void {
        globalVar = 100;
    }
}

function main(): int {
    globalVar = 0;
    Test();
    return globalVar == 100 ? 0 : 1
}
