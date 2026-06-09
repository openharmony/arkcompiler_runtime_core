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

{% for initializer in ["", "= undefined"] %}
{% for type in ["Any", "void", "undefined", "string | undefined", "number | undefined", "Object | undefined", "string | null | undefined", "int | undefined", "boolean | undefined"] -%}

/*---
desc: >-
    Default value of class member for types Any, void, undefined and unions with these.
---*/

class C {
    o: {{type}}{{initializer}}
    o1: string | {{type}} = "hello"
}

function main() {
  let c = new C
  arktest.assertEQ(c.o, undefined, "default value for type [{{type}}]")
  let x = c.o
  arktest.assertEQ(x, undefined, "undefined expected for type [{{type}}]")

  let y = c.o1
  arktest.assertEQ(y, "hello", "string expected for type [string | {{type}}]")
}
{%- endfor %}
{% endfor %}
