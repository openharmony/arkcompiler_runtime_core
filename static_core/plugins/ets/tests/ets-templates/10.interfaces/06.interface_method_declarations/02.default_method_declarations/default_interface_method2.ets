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

/*---
desc: Default interface method declaration.
assert: A default method of an interface does have a body represented by a block of code, which provides a default implementation for any class that implements the interface without overriding the method.
---*/

interface Vehicle {
  getHorsePower(rpm: int, torque: int): int {
    return (rpm * torque) / 5;
  }
}

class A implements Vehicle {
  // default interface method overridden
  public getHorsePower(rpm: int, torque: int): int {
    return rpm * torque;
  }
}

function main(): void {
  let a: A = new A();

  assert a.getHorsePower(5, 5) == 25;
}
