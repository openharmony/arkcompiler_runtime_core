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
desc: Private members of an exported class may rely on inferred types.
---*/

'use static'

// 导出类中的私有成员不在"accessible part"中，不需要显式类型
export class Counter {
    private count = 0  // OK: private field, implicit type allowed
    private reset() {  // OK: private method, implicit return type allowed
        this.count = 0
    }
    public increment(): void {
        this.count++
    }
    public getValue(): int {
        return this.count
    }
}

function main() {
    let counter = new Counter()
    counter.increment()
    arktest.assertEQ(counter.getValue(), 1)
}
