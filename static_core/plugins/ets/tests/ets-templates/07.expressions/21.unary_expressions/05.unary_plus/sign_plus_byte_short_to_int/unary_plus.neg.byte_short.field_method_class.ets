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

{% yaml "unary_plus.byte_short.test_data.yaml" as td%}
{% for c in td.testData %}
/*---
desc: >-
    {{c.desc}} Negative test (field of class). 
    When using the unary plus operator for operations on short and byte types, conversion to the int type occurs. 
    Attempting to assign this converted value to a variable of the original type (byte or short) results in a compile-time error.
expected_out: >-
    Semantic error ESE0318: Type '{{c.resultType | capitalize}}' cannot be assigned to type '{{c.type | capitalize}}'
tags: [negative, compile-only]
---*/

class A {
    fieldA: {{c.type}}
    methodA(){
        this.fieldA = {{c.format | format(c.value)}}
    }
}

function main(): void {}
{% endfor %}

