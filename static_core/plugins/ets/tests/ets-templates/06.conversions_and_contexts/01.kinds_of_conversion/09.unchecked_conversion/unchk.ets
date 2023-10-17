/*---
Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
    Let G name a generic type declaration with n type parameters.
    There is an unchecked conversion from the raw class or interface type G
    to any parameterized type of the form G<T1,...,Tn>.
    There is an unchecked conversion from the raw array type G[]k to any array
    type of the form G<T1,...,Tn>[]k.
---*/

{{c.defs}}

function main(): int {
  let src: {{c.stype}} = {{c.val|safe}};
  let dst: {{c.dtype}} = src as {{c.dtype}};
  if (src !== dst ) {  // reference equality
      return 1;
  }
  return 0;
}
{% endfor %}
