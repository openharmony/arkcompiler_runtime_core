/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Test joint type.
---*/

function main(): void {
    type T1 = string;
    type T2 = number;
    type T3 = boolean;
    type T4 = null;
    type UnionType = (T1 | T2) | (T3 | T4);

    let a: UnionType = "test";
    assertEQ(typeof a, "string");
    let b: UnionType = 10;
    assertEQ(typeof b, "number");
    let c: UnionType = true;
    assertEQ(typeof c, "boolean");
    let d: UnionType = null;
    assertEQ(typeof d, "object");
}