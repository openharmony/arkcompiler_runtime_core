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
desc: Single export directive allows specifying the declaration to be exported from the current compilation unit
files: ['./pkg01/a.ets', './pkg01/b.ets', './pkg01/c.ets', './pkg01/d.ets']
---*/

import { ClassA, InterfaceA, foo, BAR } from './pkg01'

function main() {
    assertEQ(BAR, 146)
}
