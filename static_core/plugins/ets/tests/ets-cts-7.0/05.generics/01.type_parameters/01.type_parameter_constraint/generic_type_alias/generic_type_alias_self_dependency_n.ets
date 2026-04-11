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

{% for c in cases %}
{% for cm in top_level_class_modifiers %}
/*---
desc: A compile-time error occurs if a type parameter in the type parameter section depends on itself.
params: >-
    {{c.self_dependency}}
    {{cm}}
expected_out: >-
    {{c.expected_out}}
tags: [compile-only,negative]
---*/
class SomeType {}
type T = SomeType

class SomeType2<Z> {}
type T2<Z> = SomeType2<Z>

class SomeType3<Z> {}
type T3<Z> = SomeType3<Z>

class SomeType4<Z> {}
type T4<Z> = SomeType4<Z>

{{cm}} class A<{{c.self_dependency}}> {}

{% endfor %}
{% endfor %}
