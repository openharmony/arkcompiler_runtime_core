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
desc: Diamond dependency in simultaneous - top combines both branches
tags: [not-a-test]
name: lib_top
---*/

import {LeftBranch} from './lib_left'
import {RightBranch} from './lib_right'

export function combine(): string {
    let l = new LeftBranch()
    l.id = 1
    let r = new RightBranch()
    r.id = 2
    return l.info() + "," + r.info()
}
