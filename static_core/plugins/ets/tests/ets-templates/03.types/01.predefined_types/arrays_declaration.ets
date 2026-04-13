/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: >-
    Predefined types array declaration.
---*/

{%- for i in default_value %}
let x{{loop.index}}{% if i.type %}: {{i.type}}{% endif %}{% if i.value %} = {{i.value}}{% endif %};
{%- endfor %}

function main(): void {
    {% for i in default_value %}
    let y{{loop.index}}{% if i.type %}: {{i.type}}{% endif %}{% if i.value %} = {{i.value}}{% endif %}
    {%- endfor %}
}
