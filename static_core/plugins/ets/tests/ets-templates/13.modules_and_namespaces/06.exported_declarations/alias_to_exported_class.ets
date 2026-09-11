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
desc: Exported type aliases may resolve to an exported class.
---*/

'use static'

// 类型别名指向已导出的类，应编译成功
export class Point {
  public x: double = 0.0
  public y: double = 0.0
}
export type MyPoint = Point

function main() {
    let point: MyPoint = new Point()
    point.x = 3.0
    arktest.assertEQ(point.x, 3.0)
}
