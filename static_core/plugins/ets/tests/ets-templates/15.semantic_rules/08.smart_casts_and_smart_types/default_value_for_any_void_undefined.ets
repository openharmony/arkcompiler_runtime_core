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
/*---
desc: >-
    Smart type for default value of local variable for types Any, void, undefined and unions with these.
---*/

function test1(): int {
  let testsuite = new arktest.ArkTestsuite("types Any, void, undefined and unions with these")

  {% for type in ["Any", "void", "undefined", "string | undefined", "number | undefined", "Object | undefined", "string | null | undefined", "int | undefined"] -%}

    testsuite.addTest("Test type: {{type}}", () => {
      let o: {{type}}{{initializer}}
      arktest.assertEQ(o, undefined, "default value for type [{{type}}]")
      let x: undefined = o
      arktest.assertEQ(x, undefined, "undefined via smart type for type [{{type}}]")
    });

  {%- endfor %}

  return testsuite.run()
}

function test2() {
  let x: string | undefined{{initializer}}
  x = "hello"
  let y: string = x
  arktest.assertEQ(y, "hello", "string via smart type after default undefined")
}

function main(): int {
  test2()
  return test1()
}
{% endfor %}
