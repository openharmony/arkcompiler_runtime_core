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

{% for qname in qualified_name %}
/*---
desc: Qualified name {{qname}}
params: {{qname}}
---*/

package test;

class Clazz{{loop.index0}} {
    static staticField: int = 1;
    static staticMethod(): int { return 2; }

    field: int = 3;
    method(): int { return 4; }
}

let instance{{loop.index0}}: Clazz{{loop.index0}} = new Clazz{{loop.index0}}();

{% if loop.last %}
function main(): void {
    {% for qname1 in qualified_name %}
	assertTrue( {{qname1}} )
    {% endfor %}
}
{% endif %}
{% endfor %}
