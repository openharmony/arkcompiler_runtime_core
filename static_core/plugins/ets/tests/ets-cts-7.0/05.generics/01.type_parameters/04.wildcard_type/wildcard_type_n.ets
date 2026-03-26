/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: |-
    Wildcard Type is a type that can be used as a type argument when Generic Instantiations occur to denote an unknown
    instantiation of a invariant Type Parameter. Wildcard Type is a semantic notion that may appear during the Type
    Inference, it can not be represented with ArkTS syntax.
tags: [compile-only, negative]
expected_out: {{c.expected_out}}
---*/

{{c.decl}}

function main() {
    {{c.use}}
}

{% endfor %}
