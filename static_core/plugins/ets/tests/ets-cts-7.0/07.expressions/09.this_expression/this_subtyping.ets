/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Subtyping with this expression.
assert: The class of the actual object referred to at runtime can be T if T is a class type, or a subclass of T.
---*/

class BaseClass {
  method(): BaseClass {
    return this;
  }
}

class DerivedClass extends BaseClass {
  override method(): BaseClass {
    return this; // this is DerivedClass, but assignable to BaseClass
  }
}

function main(): void {
  let base: BaseClass = new BaseClass();
  let baseResult: BaseClass = base.method();
  arktest.assertEQ(baseResult, base);
  arktest.assertTrue(baseResult instanceof BaseClass);

  let derived: DerivedClass = new DerivedClass();
  let derivedResult: BaseClass = derived.method();
  arktest.assertEQ(derivedResult, derived);
  arktest.assertTrue(derivedResult instanceof DerivedClass);
  arktest.assertTrue(derivedResult instanceof BaseClass);

  // Polymorphism test: call method through base reference
  let polyBase: BaseClass = new DerivedClass();
  let polyResult: BaseClass = polyBase.method();
  arktest.assertEQ(polyResult, polyBase);
  arktest.assertTrue(polyResult instanceof DerivedClass);
}
