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

{% for c in cases %}
/*---
desc: A smaller type's value can be assigned to a larger type's variable.
params: {{c.sm.split(" ").0}} to {{c.lg.split(" ").0}}
---*/

function main(): void {
    let smaller: {{c.sm}}
    {%- for op in  c['ops'] %}
    {
        let larger: {{c.lg}}
        larger {{op.op}} smaller
        assertTrue( larger == {{op.val}} )
    }
    {%- endfor %}
}
{% endfor %}
