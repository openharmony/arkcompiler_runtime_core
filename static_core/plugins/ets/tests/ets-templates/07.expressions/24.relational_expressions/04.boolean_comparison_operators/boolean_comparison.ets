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

{% for case in boolean_comparison %}

/*---
desc: Boolean comparison {{case.left}} {{case.op}} {{case.right}}
---*/

function main(): void {
    let x: boolean = {{case.xvalue}}
    let bx: Boolean = new Boolean({{case.xvalue}})
    let y: boolean = {{case.yvalue}}
    let by: Boolean = new Boolean({{case.yvalue}})

    assertEQ( ( {{case.xvalue}} {{case.op}} {{case.yvalue}} ), {{case.result}} )
    assertEQ( ( x {{case.op}} y), {{case.result}} )
    assertEQ( ( bx {{case.op}} by), {{case.result}} )
    assertEQ( ( x {{case.op}} by), {{case.result}} )
}

{% endfor %}
