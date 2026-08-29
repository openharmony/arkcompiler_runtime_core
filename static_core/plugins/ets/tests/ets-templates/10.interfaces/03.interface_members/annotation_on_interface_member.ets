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
desc: Annotations can be applied to interface members
tags: [compile-only]
params: "TEST-UNIT-10-0004; GAP-10-0004; CI-10-03-001-01"
assertion: "Interface members can have annotations"
test_suffix: "01"
---*/

@Retention({policy: 'SOURCE'})
@interface MemberMark {
  note: string
}

interface Annotated {
  @MemberMark({note: "property annotation"})
  value: number

  @MemberMark({note: "method annotation"})
  method(): void
}

function main(): void {
  // Compile-only test: annotations on interface members are allowed
}
