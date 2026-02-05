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
  If type used in a context is an interface that is not a superinterface
  of Array, a compile-time error occurs. Also, if each expression is not
  assignable to T in a generic superinterface I<T>, a compile-time error
  occurs.
tags: [compile-only, negative]
expected_out: 'SomeI is not a superinterface of Array'
---*/

interface SomeI {}

function main() {
    let a = [1, 2] as SomeI
}
