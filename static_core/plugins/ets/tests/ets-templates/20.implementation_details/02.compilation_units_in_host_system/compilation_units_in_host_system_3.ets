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
desc: Multi-package module independent storage.
files:
- ./pkg2/compilation_units_in_host_system_export1.ets
- ./pkg3/compilation_units_in_host_system_export2.ets
---*/

import {sum} from "./pkg2/compilation_units_in_host_system_export1.ets"
import {sum as sum0} from "./pkg3/compilation_units_in_host_system_export2.ets"

function main(): void {
    assertEQ(sum(1, 2), 3);
    assertEQ(sum0(1, 2), 3);
}