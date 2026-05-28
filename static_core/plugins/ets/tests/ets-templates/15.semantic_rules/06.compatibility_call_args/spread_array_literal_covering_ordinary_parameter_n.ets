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
desc: A spread array literal cannot cover an ordinary parameter before a rest parameter.
tags: [negative, compile-only]
expected_out: "Semantic error ESE0121: Spread argument cannot be passed for ordinary parameter."
---*/

function foo(p1: number, ...p2: number[]): void {}

function main() {
    foo(...[1, 2, 3])
}
