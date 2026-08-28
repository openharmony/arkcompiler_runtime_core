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
desc: Partial preserves covariance when U is a subtype of T.
tags: [compile-only]
---*/
// Traceability: TEST-UNIT-03-0086; GAP-03-0118; GRC-03-0153; CI-03-21-03-008-01.

class Source {
  public value: int = 1
}

class Derived extends Source {
  public extra: string = 'derived'
}

function forward(value: Partial<Derived>): void {
  let target: Partial<Source> = value
  target
}

function main(): void {
  let source: Partial<Derived> = { value: 1, extra: 'derived' }
  forward(source)
}
