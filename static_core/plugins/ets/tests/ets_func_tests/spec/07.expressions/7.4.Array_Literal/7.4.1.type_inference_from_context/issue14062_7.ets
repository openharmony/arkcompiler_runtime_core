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

/*---
desc: 7.5.1 Type inference from context
name: issue14062_7.ets
tags: [compile-only]
---*/

type VF = () => int;


class Test {

    foo(data: VF[]): int {
        return 0;
    }

}

function main(): int {
    let t: Test = new Test();

    let fd2 = () => 2;
    let fd3 = () => 3;

    return t.foo([() => 1, fd2, fd3, () => 4]);
}
