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
desc: Old lib version
tags:
- compile-only
- not-a-test
files: ['../building_blocks/method_signatures.ets', '../building_blocks/param_interfaces.ets']
---*/

import * as MS from '../building_blocks/method_signatures'
import * as PI from '../building_blocks/param_interfaces'

export class C1 implements MS.I1, MS.I2 {
    m(p: PI.PI5): string {
        return "C1"
    }
}
