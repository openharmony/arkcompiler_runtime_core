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
desc: 03.8.10 Tuple Types
name: tuple_as_param_0.ets
tags: []
---*/

function receiver(tuple: [number, string]): void {
    assertEQ( tuple[0], 123.0)
    assertEQ( tuple[1], "123.0")
}
function main(): void {
    let tt: [number, string] = [123.0, "123.0"];
    receiver(tt);
}
