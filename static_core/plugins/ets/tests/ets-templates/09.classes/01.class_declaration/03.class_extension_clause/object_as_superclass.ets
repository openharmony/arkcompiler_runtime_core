/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: Check that Object is default superclass.
assert: Given a (possibly generic) class declaration C<F1,...,Fn> (n ≥ 0, C ≠ Object), the direct superclass of the class type C<F1,...,Fn> is the type given in the extends clause of the declaration of C if an extends clause is present, or Object otherwise.
---*/

final class A<T, G> {

}

function main(): void {
  let a: Object = new A<String, Boolean>()
}
