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
    Middle of 3-package NAMED re-export chain. Selectively re-exports
    Container and unbox from lib_c, deliberately omits MAGIC so we can confirm
    the multimap forwards only what's listed.
tags: [not-a-test, compile-only]
name: lib_b
arktsconfig: arktsconfig-b.json
es2panda_options: ['emit-declaration']
---*/

export { Container, unbox } from '@panda/pkgc/lib_c'
