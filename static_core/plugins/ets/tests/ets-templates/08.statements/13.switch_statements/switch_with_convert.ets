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

{% for swc in cases %}

{% set init = swc.init | default(5) %}
{% set matchInit = swc.matchInit | default(5) %}

/*---
desc: Switch statement
---*/

function main() {
    let result: int = -1

    let x: {{swc.xType}} = {{init}}
    const match: {{swc.matchType}} = {{matchInit}}

    switch({{swc.expression}}) {

        case {{swc.caseExpression}}:
            result = 0
            break;

    }
    arktest.assertEQ(0, result)
}

{% endfor %}
