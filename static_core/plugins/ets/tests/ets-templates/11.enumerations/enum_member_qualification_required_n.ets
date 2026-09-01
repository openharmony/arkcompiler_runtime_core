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
desc: "TEST-UNIT-11-0008 GAP-11-0008 GRC-11-0008 CI-11-00-006-01: Enum member access requires type name qualification"
tags: [compile-only, negative]
expected_out: "Semantic error ESE0143: Unresolved reference Red"
---*/
enum Color { Red, Green, Blue }

function main(): void {
  let c: Color = Red  // Should fail - unqualified access
}
