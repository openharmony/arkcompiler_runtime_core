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
  Class members declared protected or public are inherited by all subclasses
  and accessible for all subclasses. Members declared private are not
  accessible to subclasses. Members include fields, methods, and accessors.
---*/

class Parent {
  public pubField: int = 1;
  protected protField: int = 2;
  private privField: int = 3;

  public pubMethod(): int { return 10; }
  protected protMethod(): int { return 20; }
  private privMethod(): int { return 30; }

  private _pubAcc: int = 100;
  public get pubAccessor(): int { return this._pubAcc; }
  public set pubAccessor(v: int) { this._pubAcc = v; }

  private _protAcc: int = 200;
  protected get protAccessor(): int { return this._protAcc; }
  protected set protAccessor(v: int) { this._protAcc = v; }
}

class Child extends Parent {
  checkProtectedField(): int { return this.protField; }
  checkProtectedMethod(): int { return this.protMethod(); }
  checkProtectedAccessor(): int { return this.protAccessor; }
  setProtectedAccessor(v: int) { this.protAccessor = v; }
}

class GrandChild extends Child {
  checkGrandProtField(): int { return this.protField; }
  checkGrandProtMethod(): int { return this.protMethod(); }
  checkGrandProtAccessor(): int { return this.protAccessor; }
}

function main(): int {
  let suite = new arktest.ArkTestsuite("Inherited members accessibility");

  suite.addTest("Public field inherited by subclass", () => {
    let c = new Child();
    arktest.assertEQ(c.pubField, 1);
  });

  suite.addTest("Public method inherited by subclass", () => {
    let c = new Child();
    arktest.assertEQ(c.pubMethod(), 10);
  });

  suite.addTest("Public accessor inherited by subclass", () => {
    let c = new Child();
    arktest.assertEQ(c.pubAccessor, 100);
    c.pubAccessor = 111;
    arktest.assertEQ(c.pubAccessor, 111);
  });

  suite.addTest("Protected field accessible in subclass", () => {
    let c = new Child();
    arktest.assertEQ(c.checkProtectedField(), 2);
  });

  suite.addTest("Protected method accessible in subclass", () => {
    let c = new Child();
    arktest.assertEQ(c.checkProtectedMethod(), 20);
  });

  suite.addTest("Protected accessor accessible in subclass", () => {
    let c = new Child();
    arktest.assertEQ(c.checkProtectedAccessor(), 200);
    c.setProtectedAccessor(222);
    arktest.assertEQ(c.checkProtectedAccessor(), 222);
  });

  suite.addTest("Public field inherited by grandchild", () => {
    let g = new GrandChild();
    arktest.assertEQ(g.pubField, 1);
  });

  suite.addTest("Public method inherited by grandchild", () => {
    let g = new GrandChild();
    arktest.assertEQ(g.pubMethod(), 10);
  });

  suite.addTest("Public accessor inherited by grandchild", () => {
    let g = new GrandChild();
    arktest.assertEQ(g.pubAccessor, 100);
  });

  suite.addTest("Protected field accessible in grandchild", () => {
    let g = new GrandChild();
    arktest.assertEQ(g.checkGrandProtField(), 2);
  });

  suite.addTest("Protected method accessible in grandchild", () => {
    let g = new GrandChild();
    arktest.assertEQ(g.checkGrandProtMethod(), 20);
  });

  suite.addTest("Protected accessor accessible in grandchild", () => {
    let g = new GrandChild();
    arktest.assertEQ(g.checkGrandProtAccessor(), 200);
  });

  return suite.run();
}
