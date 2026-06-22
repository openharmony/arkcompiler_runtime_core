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
    Export-side crash repro: a type alias nested 4500 levels deep. The parser
    recursion guard (MAX_RECURSION_DEPTH=5120) only protects PARSING; later
    phases (abc emission/codegen) recurse without a guard and overflow the
    native stack, so es2panda SEGFAULTS while compiling/exporting this
    library (rc=139), before the 5120 parser limit is reached. Empirically
    the crash starts around depth ~4000.
tags: [not-a-test, compile-only]
name: lib_deepnest_export_4500
arktsconfig: arktsconfig-deepnest_export_4500-lib.json
es2panda_options: ['emit-declaration']
---*/
export class Anchor { v: int = 1 }
export type DeepNest4500 = {% for _ in range(4500) %}Array<{% endfor %}int{% for _ in range(4500) %}>{% endfor %}
