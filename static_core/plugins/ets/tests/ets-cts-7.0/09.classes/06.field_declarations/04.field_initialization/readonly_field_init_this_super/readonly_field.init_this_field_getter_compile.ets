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
{% yaml "readonly_field.this_super.data.yaml" as yaml_file %}
{% for c in yaml_file.cases %}
/*---
desc: Checking for initialization with field initializer with usage this (other readonly field of same class and getter).
expected_out: >- 
    Warning W0010: The instance field initializer expression cannot use the this.
---*/

class A {
    private readonly _v: {{c.type}} = {{c.check_value}}
    private readonly _field: {{c.type}} = this._v
    get field(): {{c.type}} {
        return this._field
    }
}

function main(): void {
    let val = new A
    arktest.assertTrue(val.field instanceof {{c.type}}, "Expected type - {{c.type}}, actual type - " + (typeof val.field))
    arktest.assertEQ(val.field, {{c.check_value}})
}

{% endfor %}
