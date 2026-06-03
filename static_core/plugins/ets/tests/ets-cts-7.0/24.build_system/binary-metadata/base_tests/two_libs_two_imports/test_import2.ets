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
desc: Simple import lib
files: [test_lib12.ets]
tags: [compile-only]
name: test_import
arktsconfig: arktsconfig-import2.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "HI"
        - "5"
        - "HI"
---*/

import {foo} from '@panda/lib1/test_lib1'
import {NS1 as NS} from '@panda/lib1/test_lib2'
import {foo2} from '@panda/lib1/test_lib12'

foo("HI")

NS.bar({fld: 5})

foo2("HI")
