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
desc: Simple import lib789
tags: [not-a-test, compile-only]
name: l7
arktsconfig: arktsconfig-789.json
es2panda_options: ['emit-metadata', 'simultaneous']
---*/

import {E} from "@panda/lib1/l1"
import { Five } from "@panda/lib5/l5"

export function Six(): int {
    return Five(E.Four);
}
