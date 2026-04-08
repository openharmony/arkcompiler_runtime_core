/*---
Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

{% for l in literals %}
/*---
desc: >-
    Literal types are aligned with some ArkTS literals (see Literals).
    Their names are same as the names of their values, i.e., literals.
    Only three literal types are supported - string, null and undefined.
    Test literals that can be assigned but can't be used on the right
    hand side of an instanceof' expression.
tags: [negative, compile-only]
expected_out: {{l.exp_out}}
---*/

function main(): void {
    let a = {{l.literal}}
    arktest.assertTrue( a instanceof {{l.literal}} )
}

{% endfor %}
