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
desc: ReturnType independently matches the declared return type of a function.
tags: [compile-only]
---*/
// Traceability: TEST-UNIT-03-0091; GAP-03-0123; GRC-03-0160; CI-03-21-07-001-01.

function declaredInt(): int {
  return 42
}

function declaredString(): string {
  return 'return'
}

type DeclaredIntReturn = ReturnType<() => int>
type DeclaredStringReturn = ReturnType<() => string>

function main(): void {
  let fromIntSignature: DeclaredIntReturn = declaredInt()
  let fromStringSignature: DeclaredStringReturn = declaredString()
  fromIntSignature
  fromStringSignature
}
