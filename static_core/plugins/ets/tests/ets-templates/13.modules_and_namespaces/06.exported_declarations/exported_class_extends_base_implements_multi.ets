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
desc: An exported class may extend an exported base and implement multiple exported interfaces.
---*/

'use static'

// 导出类继承导出基类并实现多个导出接口，应编译成功
export class Base {
  public id: int = 0
}
export interface Serializable {
    serialize(): string
}
export interface Identifiable {
    getId(): int
}
export class Entity extends Base implements Serializable, Identifiable {
    serialize(): string {
        return 'Entity:' + this.id
    }
    getId(): int {
        return this.id
    }
}

function main() {
    let entity = new Entity()
    arktest.assertEQ(entity.getId(), 0)
    arktest.assertEQ(entity.serialize(), 'Entity:0')
}
