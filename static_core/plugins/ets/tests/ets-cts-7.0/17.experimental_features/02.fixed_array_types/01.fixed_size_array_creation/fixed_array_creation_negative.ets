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

{% for cc in cases %}
{% set ctype = cc.type | default('Object') %}

/*---
desc: Create FixedArray with wrong values
tags: [negative, compile-only]

{% if cc.expected_out is defined %}
expected_out: "{{cc.expected_out}}"
{% else %}
expected_out:  "Semantic error ESE0318: Type '{{cc.initType}}' cannot be assigned to type '{{cc.type}}'"
{% endif -%}

---*/

abstract class MyAbstractClass {}

interface IInterface {}

class IImpl implements IInterface {
}

type AT = {{ctype}}

function main(): void {
    {% if cc.logic is defined %}
        {{cc.logic|indent}}
    {% else %}
        let input = {{cc.value}}
        let a: FixedArray<AT> = new FixedArray<AT>(30, input)
    {% endif -%}

}

{% endfor %}
