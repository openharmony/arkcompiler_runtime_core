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
desc: stress single-dimension library; all declarations exported. Value sized so a single .ets file compiles in ~50s on a slower CI build (per-file timeout 90s).
tags: [not-a-test, compile-only]
name: lib_union
arktsconfig: arktsconfig-union-lib.json
es2panda_options: ['emit-declaration']
---*/
{% for i in range(3500) %}export class K{{ i }} { f: int = {{ i % 100 }} }
{% endfor %}export type BigUnion = {% for i in range(3500) %}K{{ i }}{% if not loop.last %} | {% endif %}{% endfor %}
