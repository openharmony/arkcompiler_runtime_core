/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for i in range(100, 500, 100) %}
{% set c_id = loop.index0 %}

{% if cases[c_id].expected_error %}
{% set expected_error -%}
expected_error: "{{cases[c_id].expected_error}}"
{%- endset -%}
{% endif %}

/*---
desc: Enums must support any number of constants
params: number of constants = {{i}}
{{expected_error}}
---*/

enum GeneratedEnum {
    {% for length in range(1, i) -%}
    {% for _ in range(length)%}A{% endfor %}{%if length is lt(i - 1)%},{% endif %}
    {% endfor %}
}

function main(): void {
  let e = GeneratedEnum.{% for _ in range(i - 1)%}A{% endfor %}
  arktest.assertTrue(e instanceof GeneratedEnum)
  arktest.assertTrue(e.valueOf() instanceof int)
  arktest.assertEQ(e, {{i - 2}})
}

{%- endfor %}
