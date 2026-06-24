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
    Library exporting single-field annotations. Each has exactly one
    property; the consumer will apply them in the IMPLICIT form
    `@Anno(x)` (without `{key: x}`).
tags: [not-a-test, compile-only]
name: lib_anno
arktsconfig: arktsconfig-lib.json
es2panda_options: ['emit-metadata']
---*/

@Retention({policy: 'RUNTIME'})
export @interface Tag {
    name: string = ""
}

@Retention({policy: 'RUNTIME'})
export @interface Level {
    n: int = 0
}
