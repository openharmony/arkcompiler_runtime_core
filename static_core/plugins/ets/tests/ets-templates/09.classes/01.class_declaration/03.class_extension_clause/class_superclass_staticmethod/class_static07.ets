/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: A subclass calls a static method of its superclass, and the superclass method returns a static property.
---*/

class Parent {
  static staticProperty: string;

  static {
    Parent.staticProperty = "Parent static property";
  }

  static staticMethod(): string {
    return Parent.staticProperty;
  }
}

class Child extends Parent {
  static callParentStaticMethod(): string {
    return Parent.staticMethod();
  }
}

function main(): void {
  const childInstance = new Child();
  assertEQ(Child.callParentStaticMethod(), "Parent static property");
}