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
files:
- ./source1.ets
- ./source2.ets
---*/

import { foo } from './source1.ets'
import { foo } from './source2.ets'

function foo(p: int): int {
    return p * 2
}

function main(): int {
    if (foo('XYZ') != 'XYZXYZ' || foo(2) != 4 || foo(1.2) != 2.4) {
        return 1
    }
    return 0
}
