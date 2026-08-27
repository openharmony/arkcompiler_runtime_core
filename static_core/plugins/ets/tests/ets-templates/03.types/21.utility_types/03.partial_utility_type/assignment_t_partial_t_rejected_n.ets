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
desc: Assignment from T to Partial<T> is rejected.
tags: [compile-only, negative]
---*/
// Traceability: TEST-UNIT-03-0084; GAP-03-0116; GRC-03-0151; CI-03-21-03-005-01.

class PartialAssignmentSource {
  public value: int = 1
}

function acceptsPartial(value: Partial<PartialAssignmentSource>): void {
  value
}

function main(): void {
  acceptsPartial(new PartialAssignmentSource())
}
