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
desc: >-
    Source declaration test: imports from simultaneous lib (@panda/lib via .etscache)
    and from .d.ets file (@panda/decl via SOURCE_DECL). Verifies that both
    ETSCACHE_DECL and SOURCE_DECL paths work in the same compilation unit.
tags: [compile-only]
files: [lib_a.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
---*/

import { Coord, distance } from '@panda/lib/lib_a'
import { Metric, formatMetric } from '@panda/decl/lib_types'

// Use types from simultaneous lib (ETSCACHE_DECL path)
let a: Coord = new Coord(0, 0)
let b: Coord = new Coord(3, 4)
let d: int = distance(a, b)

// Use types from .d.ets source declaration (SOURCE_DECL path)
let m: Metric = new Metric("cpu", 99)
let desc: string = formatMetric(m)
