/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Overriding abstract method.
assert: A compile-time error occurs if an abstract method overrides a non-abstract instance method
tags: [positive, compile-only]
---*/

abstract class A {
  abstract run(): number;
}

class B extends A {
  override run(): number {
    let result = 5 + 5;
    return result;
  };
}

function main(): void {
  let b: A = new B();
<<<<<<< HEAD:static_core/plugins/ets/tests/ets-templates/09.classes/06.method_declarations/06.overriding_methods/overriding_by_instance_methods/abstract_method_overridden_by_instance.ets
  assertEQ(b.run(), 10);
=======
  arktest.assertEQ(b.run(), 10);
>>>>>>> OpenHarmony_feature_20250328:static_core/plugins/ets/tests/ets-templates/09.classes/05.method_declarations/05.overriding_methods/overriding_by_instance_methods/abstract_method_overridden_by_instance.ets
}

