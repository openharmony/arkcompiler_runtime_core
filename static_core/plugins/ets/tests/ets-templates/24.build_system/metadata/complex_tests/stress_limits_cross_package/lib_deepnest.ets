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
    Deeply-nested type alias (depth 1000). Kept FAR below the emission stack-overflow
    threshold (~2500-3000 at an 8MB stack; this positive test must NOT crash). The companion
    Anchor class is what the consumer actually uses.
tags: [not-a-test, compile-only]
name: lib_deepnest
arktsconfig: arktsconfig-deepnest-lib.json
es2panda_options: ['emit-declaration']
---*/
export class Anchor { v: int = 1 }
export type DeepNest = {% for _ in range(1000) %}Array<{% endfor %}int{% for _ in range(1000) %}>{% endfor %}
