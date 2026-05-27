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
    If the result of x instanceof T is false, then x as T causes ClassCastError thrown at runtime
expected_out: "Warning W0326: Attempting to cast type 'B' to 'A' can throw 'ClassCastError' exception at runtime"
---*/

class A {}
class B {}

function main(): void {
  let b: B = new B()
  arktest.expectThrow(() => { let a = b as A }, (e) => e instanceof ClassCastError);
}
