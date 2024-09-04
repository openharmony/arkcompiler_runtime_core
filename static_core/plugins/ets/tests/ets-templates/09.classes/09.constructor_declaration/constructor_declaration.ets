/*---
Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

{% for mod in modifiers %}

/*---
desc: Class constructor declaration
assert: A constructor is used in the creation of an object that is an instance of a class. A constructor declaration starts with the constructor keyword and does not has a name. In all other respects, a constructor declaration looks just like a method declaration that has no result.
params: {{mod}}
tags: [compile-only]
---*/

class A {
  public x: int;

  {{mod}} constructor() {
    this.x = 5;
  }
}

{% endfor %}
