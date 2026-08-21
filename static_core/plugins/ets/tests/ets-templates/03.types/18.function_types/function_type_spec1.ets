/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Example for type function. A type alias can be set for a function type.
---*/

type BinaryOp = (x: number, y: number) => number

function evaluate(f: BinaryOp) {
    arktest.assertEQ(f(1.0, 2.0), 4.0)
}

let binaryOp: BinaryOp

function main() {
    binaryOp = (a: number, b: number): number => a + b + 1
    evaluate(binaryOp)
}
