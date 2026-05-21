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
desc: Derived class with package import from @panda/lib/lib_base
tags: [not-a-test, compile-only]
name: lib_child_pkg
arktsconfig: arktsconfig-child-pkg.json
files: [lib_base.ets]
es2panda_options: ['emit-declaration']
---*/

import { Base } from '@panda/lib/lib_base'

export class Child extends Base {
    constructor(value: string) {
        super(value)
    }
    getLabel(): string {
        return "child:" + this.getValue()
    }
}
