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
{%- for c in cases %}
/*---
desc: Tests for effective types listed in the table in chapter 15.12 spec 1.2.1a-tp10.
test_suffix: "{{c.test_suffix}}_{{loop.index}}"
---*/
{{c.decl}}

function verify_effective_type(value: Any, kind:string) {
  arktest.assertTrue(value instanceof ({{c.effective_type}}), "expect instance of {{c.effective_type}} " + kind)
}

function main() {
    let variable:{{c.tested_type}} {{c.init}}
    verify_effective_type(variable, "variable")
    const constant:{{c.tested_type}} {{c.init}}
    verify_effective_type(constant, "constant")
}
{%- endfor %}
