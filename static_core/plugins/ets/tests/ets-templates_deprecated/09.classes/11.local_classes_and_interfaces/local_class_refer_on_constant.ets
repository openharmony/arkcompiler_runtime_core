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

{% for t in variable_types %}

/*---
desc: Declaring local class in method body with refer on constant {{t}} variable.
assert: A local class can only be referred to with a simple name (not a qualified or canonical name), i.e., a local class cannot be considered if a canonical name is required.
params: {{t}}
tags: [compile-only]
---*/

class A {
  public static const x: {{t}};

  public static bar(): void {
    class B {
      public bar(): void {
        let y: {{t}} = x;
      }
    }
  }
}

{% endfor %}
