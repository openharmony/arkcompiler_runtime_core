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

{% for member in members %}
/*---
desc: Check members with protected access modifier in class.
assert: Member or constructor with protected modifier can be accessed only within their declaring class and classes derived from their declaring class.
params: "{{member.body}}"
{%- if member.unsupported == "true" %}
tags: [negative, compile-only]
{%- endif %}
---*/

class A {
  protected {{member.body}}

  public call(): {{member.type}} {
    return this.{{member.call}};
  }
}


function main(): void {
  let instance: A = new A();

  assertEQ( instance.call(), {{member.value}} );
}

{% endfor %}
