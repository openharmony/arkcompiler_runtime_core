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
desc: Pass FixedArray as Array parameter
tags:
- negative
- compile-only
expected_err: "Semantic error ESE0127: No matching call signature for processResize(FixedArray<Double>)Semantic error ESE0046: Type 'FixedArray<Double>' is not compatible with type 'Array<Double>' at index 1"
---*/

function processResize(arr: Array<number>): void {
  /** Some operations **/
}

let arr1: FixedArray<number> = [1, 2, 3];

function main(): void {
  processResize(arr1);
}
