/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: A name must be unique in its space
tags: [negative, compile-only]
expected_out: 
  compiler: 
  - "Semantic error ESE0351: Variable 'x' has already been declared."
  - "Semantic error ESE0351: Variable 'y' has already been declared."
  - "Semantic error ESE0351: Variable 'z' has already been declared."
  - "Semantic error ESE0351: Variable 'k' has already been declared."
  - "Semantic error ESE0351: Variable 'j' has already been declared."
  - "Semantic error ESE0351: Variable 'V1' has already been declared."
  - "Semantic error ESE0351: Variable 'B' has already been declared."
  - "Semantic error ESE0351: Variable 'Enk' has already been declared."
  - "Semantic error ESE0130: Function f is already declared. "
---*/

enum En {V1, V1}

enum Enk {H1, H2}
enum Enk {H3, H4}

function f(): void {}
function f(): void {}

class B {}
class B {}

interface A {
    x: int;
    x: float;
}

class A {
    y: int;
    y: int;
}

{
    let z = 0;
    let z = 0;
}

function foo(): void {
    {
        let k = 1;
        let k = 1;
    }
}

function main(): void {
    {
        let j = 0;
        let j = 0;
    }
}
