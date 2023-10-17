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
desc: Private interface methods.
assert: A default method in an interface body may explicitly be declared private.
---*/
interface Vehicle {
  private getHorsePower3(rpm: int, torque: int): int {
      return (rpm * torque) / 5252;
  }

  getPwrIndex(torque: int): int {
      return this.getHorsePower3(5252, torque);
  }
}

class Car implements Vehicle {}

function main(): void {
    assert new Car().getPwrIndex(1) == 1
}
