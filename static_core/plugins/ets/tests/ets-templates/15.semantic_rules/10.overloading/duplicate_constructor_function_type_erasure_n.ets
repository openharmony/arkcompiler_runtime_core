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
desc: Duplicate constructor function type erasure is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Warning W2323: The order of entities in an overload set implies that the overloaded entity
        with the signature '(p: ((p1: Double) => String)) => undefined' will never be selected for a
        call, as the preceding entity with the signature '(p: Object) => undefined' prevails.
    - >-
        Semantic error ESE0131: Function constructor with this assembly signature already declared.
    - >-
        Warning W2323: The order of entities in an overload set implies that the overloaded entity
        with the signature '(p: ((p1: String) => Double)) => undefined' will never be selected for a
        call, as the preceding entity with the signature '(p: Object) => undefined' prevails.
---*/

'use static'

type FF=(p: number) => string
type EE=(p: string) => number
class Ex extends Error {}
let ex: Ex = new Ex()
class A {
  public t: string = ''

  getType(): string {
    return this.t
  }

  constructor() { throw ex }
  constructor(p: Object) { this.t = 'should never be called' }
  constructor(p: FF) {
    if (typeof p == this.t) {
      throw ex
    } else {
      this.t = 'FF'
    }
  }
  constructor(p: EE) { this.t = 'EE' }
}
