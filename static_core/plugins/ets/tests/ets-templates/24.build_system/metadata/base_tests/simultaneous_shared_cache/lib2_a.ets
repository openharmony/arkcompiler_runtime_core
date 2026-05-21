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
desc: Shared cache - second batch file A (imports from first batch)
tags: [not-a-test, compile-only]
files: [lib1_a.ets]
name: lib2_a
arktsconfig: arktsconfig-lib2.json
es2panda_options: ['emit-declaration', 'simultaneous']
---*/

import {Beta} from '@panda/lib1/lib1_b'

export class Gamma {
    getBetaVal(): int {
        return new Beta().val
    }
}
