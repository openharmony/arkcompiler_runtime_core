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

/*---
desc: A private field from of a superclass access, if both classes are members of same class.
assert: A private field of a superclass might be accessible to a subclass - for example, if both classes are members of the same class.
---*/

class A {
  inner class C {
    private static x: int;
  }

  inner class B extends C {
    public getX(): int {
      return C.x;
    }
  }

  public getValue(): int {
    return new B.getX();
  }
}



function main(): void {
    let instance: A = new A();

    assert instance.getValue() == 0;
}
