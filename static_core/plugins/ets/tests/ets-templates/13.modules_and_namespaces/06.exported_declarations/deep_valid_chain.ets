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
desc: An exported class may extend an exported class through multiple levels.
---*/

'use static'

// 多层继承链：所有参与的类都是导出的，应编译成功
export class GrandParent {
    public a: int = 1
}
export class Parent extends GrandParent {
  public b: int = 2
}
export class Child extends Parent {
  public c: int = 3
}

function main() {
    let child = new Child()
    arktest.assertEQ(child.a + child.b + child.c, 6)
}
