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
desc: >-
  If a variable or constant initializer expression refers to a variable or a constant, and the
  declaration of that variable or constant is textually located after the current declaration,
  then a compile-time error occurs.
tags: [compile-only, negative]
expected_out: 'Invalid initializer'
---*/


{{c.decl}}

function main(): void {
  arktest.assertTrue(false, "this code is not supposed to execute")
}
{% endfor %}
