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
desc: Type T is rejected as a subtype of Required<T> in the specified direction.
tags: [compile-only, negative]
---*/
// Traceability: TEST-UNIT-03-0088; GAP-03-0120; GRC-03-0155; CI-03-21-04-006-01.

interface RequiredSpecifiedSource {
  value?: int
}

function requiresRequiredSubtype<T extends Required<RequiredSpecifiedSource>>(value: T): void {
  value
}

function main(): void {
  let source: RequiredSpecifiedSource = {}
  requiresRequiredSubtype<RequiredSpecifiedSource>(source)
}
