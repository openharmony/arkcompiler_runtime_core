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
desc: 05.Generics
name: 05.generics/generic_01
tags: []
---*/

function foo<T>(arg1: T): void {
    const cb: (arg: T) => void = (arg: T) => {
        assertTrue( arg instanceof string)
    }
    cb(arg1);
}

function main(): void {
    foo<string>("F");
}
