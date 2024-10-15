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
desc: Access to field by keyword super from constructor of Object class
assert: It is a compile-time error if the forms using the keyword super appear in the declaration of class Object, since Object has no superclass.
---*/

package std.core;

class Object {
  constructor() {
    super()
  }

  public toString(): String {
    return "";
  }

  public $_hashCode(): int {
    return runtime.GetHashCode(this);
  }

  public equals(to: Object): boolean {
    return runtime.equals(this, to);
  }
}
