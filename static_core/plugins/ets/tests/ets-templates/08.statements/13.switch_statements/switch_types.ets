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

{% for item in switch_statements %}

/*---
desc: Switch expression can be of any type
---*/

function f(i: {{item.type}}): void {
  switch (i) {
    case {{item.param1}} as {{item.type}}:
      arktest.assertEQ(i, {{item.param1}})
      break;
    case {{item.param2}} as {{item.type}}:
      arktest.assertEQ(i, {{item.param2}})
      break;
    default:
      arktest.assertEQ(i, i)
  }
}

function main(): void {
  f({{item.param1}});
  f({{item.param2}});
}

{% endfor %}
