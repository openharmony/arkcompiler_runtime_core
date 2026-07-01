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
 desc: Issue 34667. Bytecode Optimizer. Opcode Compare not yet implemented
 tags: []

 ---*/

function main() {
    let ARR_BOOLEAN: boolean[] = [false]
    let INT0: int = 667692309
    let BOOL0: boolean = false

    if( ARR_BOOLEAN[0] = BOOL0 = true & BOOL0 &!! BOOL0 && true ^! BOOL0);
    if( true &! BOOL0)

    INT0 = 1
    return INT0
}
