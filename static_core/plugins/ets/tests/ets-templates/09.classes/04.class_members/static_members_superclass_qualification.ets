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
  Static members of a superclass are not inherited. The subclass must use
  explicit qualification using the superclass name to access static fields,
  methods, and accessors.
---*/

class Base {
  static sField: int = 10;

  static sMethod(): int {
    return 20;
  }

  private static _accVal: int = 30;
  static get sAccessor(): int { return Base._accVal; }
  static set sAccessor(v: int) { Base._accVal = v; }
}

class Child extends Base {}

class GrandChild extends Child {}

function main(): int {
  let suite = new arktest.ArkTestsuite("Static members explicit superclass qualification");

  suite.addTest("Access static field via superclass name", () => {
    arktest.assertEQ(Base.sField, 10);
  });

  suite.addTest("Access static method via superclass name", () => {
    arktest.assertEQ(Base.sMethod(), 20);
  });

  suite.addTest("Access static getter via superclass name", () => {
    arktest.assertEQ(Base.sAccessor, 30);
  });

  suite.addTest("Modify static field via superclass name", () => {
    Base.sField = 15;
    arktest.assertEQ(Base.sField, 15);
    Base.sField = 10;
  });

  suite.addTest("Modify static setter via superclass name", () => {
    Base.sAccessor = 35;
    arktest.assertEQ(Base.sAccessor, 35);
    Base.sAccessor = 30;
  });

  suite.addTest("Access static field of grandparent via superclass name", () => {
    arktest.assertEQ(Base.sField, 10);
  });

  suite.addTest("Access static method of grandparent via superclass name", () => {
    arktest.assertEQ(Base.sMethod(), 20);
  });

  suite.addTest("Access static accessor of grandparent via superclass name", () => {
    arktest.assertEQ(Base.sAccessor, 30);
  });

  return suite.run();
}
