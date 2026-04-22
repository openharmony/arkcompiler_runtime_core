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

/*---
desc: >-
    If execution reaches field initialization that uses this and reads an
    uninitialized field, a runtime error can occur.
---*/

class C {
  public f0 = this
  public f1: string = this.init_f1()

  public init_f1(): string {
    console.log(this.f1)
    return "a string field"
  }
}

function main(): void {
  try {
    let instance: C = new C();
  } catch (e) {
  }
}
