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
desc: An exported generic class may extend an exported base class.
---*/

'use static'

// 导出泛型类继承导出类，类型参数本身不触发此规则
export class Base {
  public value: int = 0
}
export class Wrapper<T> extends Base {
  public item: T
    constructor(item: T) {
        this.item = item
    }
}

function main() {
    let wrapper = new Wrapper<int>(9)
    arktest.assertEQ(wrapper.item, 9)
    arktest.assertEQ(wrapper.value, 0)
}
