/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

/*---
desc: Enum constant should have a toString method that returns its value as a string. Int enum used.
---*/

enum GeneratedEnum {
    {% for length in range(1, 10) -%}
    {% for _ in range(length)%}A{% endfor %}{%if length is lt(9)%},{% endif %}
    {% endfor %}
}


function main(): void {
    {% for i in range(1, 10) %}
    assertTrue(GeneratedEnum.{% for _ in range(i)%}A{% endfor %}.toString() == "{{i-1}}");
    {%- endfor %}
}
