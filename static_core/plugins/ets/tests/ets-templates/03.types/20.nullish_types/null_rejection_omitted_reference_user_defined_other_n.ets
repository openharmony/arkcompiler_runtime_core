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

{% for case in cases %}
/*---
desc: {{case.desc}}
test_suffix: '{{case.test_suffix}}'
tags: [compile-only, negative]
expected_out: |-
    {{case.expected_out}}
---*/
// Traceability: TEST-UNIT-03-0080; GAP-03-0112; GRC-03-0144; CI-03-20-003-01.
// Case traceability: {{case.gap_id}}; {{case.grc_id}}; {{case.ci_id}}.
{{case.decl|indent}}

function main(): void {
  {{case.use|indent}}
}
{% endfor %}
