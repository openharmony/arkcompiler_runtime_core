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

{% for mod in modifiers %}
{% for member in members %}
/*---
desc: Access modifiers.
assert: Members and constructors may have either private, internal, internal protected, protected or public modifier to explicitly specify the desired accessibility.
params: "{{mod}} {{member}}"
{%- if member.unsupported == "true" %}
tags: [negative, compile-only]
{%- else %}
tags: [compile-only]
{%- endif %}
---*/

class A {
  {{mod}} {{member.body}}
}

{% endfor %}
{% endfor %}
