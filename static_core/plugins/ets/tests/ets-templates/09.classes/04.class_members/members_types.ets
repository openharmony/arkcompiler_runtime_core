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
  Members can be class fields, methods, and accessors. Static members are
  not part of class instances and are accessed via qualified name notation.
  Non-static (instance) members belong to any instance of the class.
---*/

class MyClass {
  instanceField: int = 5;

  instanceMethod(): int {
    return this.instanceField * 2;
  }

  private _prop: int = 15;
  get instanceProp(): int { return this._prop; }
  set instanceProp(v: int) { this._prop = v; }

  static staticField: int = 50;

  static staticMethod(): int {
    return MyClass.staticField * 2;
  }

  private static _sProp: int = 75;
  static get staticProp(): int { return MyClass._sProp; }
  static set staticProp(v: int) { MyClass._sProp = v; }
}

function main(): int {
  let suite = new arktest.ArkTestsuite("Class members types");

  suite.addTest("Instance field belongs to instance", () => {
    let obj1 = new MyClass();
    let obj2 = new MyClass();
    obj1.instanceField = 11;
    arktest.assertEQ(obj1.instanceField, 11);
    arktest.assertEQ(obj2.instanceField, 5);
  });

  suite.addTest("Instance method belongs to instance", () => {
    let obj = new MyClass();
    arktest.assertEQ(obj.instanceMethod(), 10);
    obj.instanceField = 7;
    arktest.assertEQ(obj.instanceMethod(), 14);
  });

  suite.addTest("Instance accessor belongs to instance", () => {
    let obj1 = new MyClass();
    let obj2 = new MyClass();
    obj1.instanceProp = 25;
    arktest.assertEQ(obj1.instanceProp, 25);
    arktest.assertEQ(obj2.instanceProp, 15);
  });

  suite.addTest("Static field accessed via class name", () => {
    arktest.assertEQ(MyClass.staticField, 50);
  });

  suite.addTest("Static method accessed via class name", () => {
    arktest.assertEQ(MyClass.staticMethod(), 100);
  });

  suite.addTest("Static accessor accessed via class name", () => {
    arktest.assertEQ(MyClass.staticProp, 75);
    MyClass.staticProp = 80;
    arktest.assertEQ(MyClass.staticProp, 80);
    MyClass.staticProp = 75;
  });

  return suite.run();
}
