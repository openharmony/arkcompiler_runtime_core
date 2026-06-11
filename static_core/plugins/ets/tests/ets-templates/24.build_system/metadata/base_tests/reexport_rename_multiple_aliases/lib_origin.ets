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
    Origin library with several symbols that the proxy will re-export
    under different names — all in a SINGLE re-export statement.
tags: [not-a-test, compile-only]
name: lib_origin
arktsconfig: arktsconfig-origin.json
es2panda_options: ['emit-declaration']
---*/

export class Original {
    val: int = 1
}

export function compute(): int {
    return 42
}

export let HANDLE: int = 7
