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
evaluation:
  expected_imports_base: True
---*/

interface A {
    foo(x: double): double;
};

interface B {
    goo(x: int): int;
}

export class C implements A, B {
    foo(x: double): double { return x };
    goo(x: int): int { return x };
};

function main(): void {
    undefined;  // #BP
    console.log("After evaluate");
    undefined;  // #BP
}
