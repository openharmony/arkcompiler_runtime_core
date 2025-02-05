/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: >-
  A variable declaration (§4.7.1) or a constant declaration (§4.7.2) expression
  used to initialize a variable or constant must not have calls to functions
  that can throw or rethrow exceptions.
tags: [compile-only, negative]
---*/

let i = 2;

function throwing_function(): int throws {
    if (i * i != 4) {
        throw new Exception();
    }
    return i;
}

function main(): int {
    try {
        const v: int = throwing_function(); // CTE expected
        return v;
    } catch (e: Error) {
        return 0;
    }
}
