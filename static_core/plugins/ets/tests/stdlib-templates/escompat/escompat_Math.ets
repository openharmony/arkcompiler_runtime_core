/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

{% for item in escompat_Math_trigonometry %}

/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_escompat_math_lib.j2' %}
{% endfor %}

{% for item in escompat_Math_arithmetic %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_escompat_math_lib.j2' %}
{% endfor %}

{% for item in escompat_Math_bits %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_escompat_math_lib.j2' %}
{% endfor %}

{% for item in escompat_Math_h_trigonometry %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_escompat_math_lib.j2' %}
{% endfor %}

{% for item in escompat_Math_log %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_escompat_math_lib.j2' %}
{% endfor %}

{% for item in escompat_Math_exp %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_escompat_math_lib.j2' %}
{% endfor %}
