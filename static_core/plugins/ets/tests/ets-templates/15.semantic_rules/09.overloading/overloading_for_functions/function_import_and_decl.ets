/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

{% for c in cases %}

/*---
desc: >-
    A function can be declared in, or imported to a scope.
    The semantic check for these functions is as follows:
    • If signatures of functions are overload-equivalent, then a compile-time error occurs.
    • Otherwise, overloading is valid.
    More details can be found in Function Overloading and in Import and Overloading of Function Names.
files: [ ./modules/{{c.file_a}}.ets {% if c.file_b %},./modules/{{c.file_b}}.ets{% endif %} ]
tags: [{{c.tags}} compile-only]
---*/

import {foo} from "./modules/{{c.file_a}}.ets"
{% if c.file_b %}
import {foo} from "./modules/{{c.file_b}}.ets"
{% endif %}

{% if c.decl %}
{{c.decl}}
{% endif %}

{% endfor %}
