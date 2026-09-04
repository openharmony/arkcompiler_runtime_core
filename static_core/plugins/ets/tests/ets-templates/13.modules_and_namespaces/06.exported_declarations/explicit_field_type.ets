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
desc: Public fields and methods of an exported class have explicit types.
---*/

'use static'

// 导出类的非私有字段和方法有显式类型/返回类型，应编译成功
export class Point {
    public x: double = 0.0
    public y: double = 0.0
    public getDistance(): double {
        return this.x * this.x + this.y * this.y
    }
}

function main() {
    let point = new Point()
    point.x = 3.0
    point.y = 4.0
    arktest.assertEQ(point.getDistance(), 25.0)
}
