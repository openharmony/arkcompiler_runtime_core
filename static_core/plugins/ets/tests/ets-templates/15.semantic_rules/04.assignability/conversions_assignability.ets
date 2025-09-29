/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
{% for case in cases %}
{% for type2 in case.type2 %}

/*---
desc: Type T1 is assignable to type T2 if implicit conversion is present
---*/

{{case.decl}}

let t1: {{case.type1}} = {{case.value}}
let t2: {{type2}} = t1

{% endfor %}
{% endfor %}
