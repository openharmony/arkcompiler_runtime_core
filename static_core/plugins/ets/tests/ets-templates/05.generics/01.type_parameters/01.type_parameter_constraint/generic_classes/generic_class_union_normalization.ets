/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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
    The constraint is denoted as any type except the case when constraint is a single final class
    (see Final Classes) or becomes a single final class as a result of union normalization
    (see Union Types Normalization). Then a compile-time error occurs.
tags: [{{c.tags}}]
---*/

{{c.decl}}

{% endfor %}
