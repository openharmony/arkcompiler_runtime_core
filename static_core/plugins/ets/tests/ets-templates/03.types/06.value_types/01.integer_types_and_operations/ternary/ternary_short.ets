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

{% for v in values %}
/*---
desc: check ternary if-else operation with condition of short integer operand
---*/

function main(): void {
    const a: short = {{v.condition}}
    const b: short = {{v.ifTrue}}
    const c: short = {{v.ifFalse}}
    assertEQ( (a ? b : c), {{v.result}})
}

{% endfor %}
