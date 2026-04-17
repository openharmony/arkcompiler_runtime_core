/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
desc: Compile-time error test for String equality
{% if case.expect_cte is defined and not case.expect_cte %}
tags: [compile-only]
{% else %}
tags: [negative, compile-only]
{% endif %}
{% if case.expected_out is defined %}
expected_out: {{case.expected_out}}
{% endif %}
---*/

class TestClass {}

function main(): void {
    let x: {{case.ltype}} = {{case.lval}}
    let y: {{case.rtype}} = {{case.rval}}
    x {{case.op}} y
}

{% endfor %}
