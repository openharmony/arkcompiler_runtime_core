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

/*---
desc: >-
  NonNullable<T> as return type in generic class method with T extends MyBase | undefined constraint.
  Instantiation with non-undefined type: result is non-null.
---*/

class MyBase {}

class A<T extends MyBase | undefined> {
    foo(p:T): NonNullable<T> {
        return p!
    }
}

function main() {
    let a = new A<MyBase>
    let result: MyBase = a.foo(new MyBase)
    arktest.assertNE(result, null);
}
