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
desc: 'The forward subtype constraint T <: Partial<T> is rejected.'
tags: [compile-only, negative]
---*/
// Traceability: TEST-UNIT-03-0085; GAP-03-0117; GRC-03-0152; CI-03-21-03-006-01.

class Source {
  public value: int = 1
}

function requiresPartialSubtype<T extends Partial<Source>>(value: T): void {
  value
}

function main(): void {
  requiresPartialSubtype<Source>(new Source())
}
