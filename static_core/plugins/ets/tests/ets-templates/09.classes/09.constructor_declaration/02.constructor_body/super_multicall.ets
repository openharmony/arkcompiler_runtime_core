/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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

/*---
desc: A constructor body can have no more than one call to the current class or direct superclass constructor
tags: [compile-only, negative]
---*/

class A {
  public i: int
  public s: string

  public constructor(i: int) {
    this.i = i
  }

  public constructor(s: string) {
    this.s = s
  }
}

class B extends A {
  public constructor() {
    super(123)
    super("Just a test")
  }
}
