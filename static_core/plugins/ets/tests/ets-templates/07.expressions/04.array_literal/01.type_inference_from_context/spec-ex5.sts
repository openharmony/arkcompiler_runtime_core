/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
  There are cases when every array or tuple element of primitive type is boxed
  (see Boxing Conversions) according to the type of the context that creates a
  new array or tuple.
---*/

function bar (tuple: [Number, Boolean]) {
    assertEQ(tuple[0].unboxed(), 1.0)
    assertEQ(tuple[1].unboxed(), true)
}

function main() {
    let tuple: [Number, Boolean] = [1, true] // assignment context
    assertEQ(tuple[0].unboxed(), 1.0)
    assertEQ(tuple[1].unboxed(), true)

    bar([1, true])  // call context

    let t: [Number, Boolean] = [1, true] as [Number, Boolean] // casting conversion
    assertEQ(t[0].unboxed(), 1.0)
    assertEQ(t[1].unboxed(), true)
}
