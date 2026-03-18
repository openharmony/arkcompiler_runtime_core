/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

{% for timing in ["compile", "runtime"] %}

/*---
desc: Non-static field 'f' have reference in non-static variable.
assert: >-
    If the instance field initializer expression uses
    this or super directly in any form,then a compile-time warning occurs.
{% if 'compile' in timing %}
tags: [compile-only]
expected_out: "Warning W0010: The instance field initializer expression cannot use the this."
{% endif %}
---*/

// #2345 after runner is made to support this properly, remove compile-only tag and "timing" loop

class C {
  public f: int
  public a: int = this.f + 5
}

class C1 {
  public f: int = this.f + 5
}

function main(): void {
  let c = new C()
  arktest.assertEQ(c.f, 0)
  arktest.assertEQ(c.a, 5)

  let c1 = new C1()
  arktest.assertEQ(c1.f, 5)
}

{% endfor %}
