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

{% for eq in char_numeric_eq %}

/*---
desc: >-
    char vs numeric type equality via widening conversion.
    {{eq.left}} ({{eq.left_type}}) {{eq.op}} {{eq.right}} ({{eq.right_type}}) is {{eq.result}}.
    Widening: {{eq.widening}}.
---*/

function main(): void {
    let left: {{eq.left_type}} = {{eq.left}}
    let right: {{eq.right_type}} = {{eq.right}}
    arktest.assertEQ(
        left {{eq.op}} right,
        {{eq.result}},
        "Expected: {{eq.left}} ({{eq.left_type}}) {{eq.op}} {{eq.right}} ({{eq.right_type}}) is {{eq.result}}"
    )
}

{% endfor %}
