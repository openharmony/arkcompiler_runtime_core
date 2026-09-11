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
desc: An exported interface may extend another exported interface.
---*/

'use static'

// 导出接口继承自另一个导出接口，应编译成功
export interface Named {
    getName(): string
}
export interface Person extends Named {
    getAge(): int
}

class PersonImpl implements Person {
    getName(): string { return 'Ada' }
    getAge(): int { return 42 }
}

function main() {
    let person = new PersonImpl()
    arktest.assertEQ(person.getName(), 'Ada')
    arktest.assertEQ(person.getAge(), 42)
}
