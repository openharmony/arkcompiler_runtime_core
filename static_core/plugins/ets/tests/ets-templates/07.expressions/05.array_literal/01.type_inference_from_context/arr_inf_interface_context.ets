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
  If type used in a context is an interface I, and I is a generic
  superinterface of a resizable array type with the single type parameter
  I<T>, then an array literal is considered as an instance of Array<T>.
---*/

function main() {
    let b: ConcatArray<number> = [1, 2]
    arktest.assertEQ(b.length, 2)

    let d: ArrayLike<Object> = [1, "aa"]
    arktest.assertEQ(d.length, 2)
}
