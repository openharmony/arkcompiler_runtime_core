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
desc: Re-export directive, new names should be available
files: ['./module_all_renamed.ets', './module_a.ets']
---*/

import { NewClassA, NewInterfaceA, newFoo, NEW_BAR } from './module_all_renamed'

function main() {
    assertEQ(NEW_BAR, 146)
}
