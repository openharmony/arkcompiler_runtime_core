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
desc: Setter throw error.
---*/

class TestException extends Exception {}

class A {
  private _age: int = 0;

  set age(value: int) throws {
    if (value < 0) { 
      throw new TestException();
    }

    this._age = value;
  }
}

function main(): void {
  let instance: A = new A();
  let catchCode = 0;

  try {
    instance.age = -5;
  } catch (e: TestException){
    catchCode = 1;
  } catch {
    assert false;
  }

  assert catchCode == 1;
}
