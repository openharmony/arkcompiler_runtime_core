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
desc: Rejects an exported declaration that exposes a non-exported type.
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE1236581: Type 'Point' used in export declaration(s) is not exported.
---*/

'use static'

// 中间类型别名未导出，但 export type 指向一个未导出的类型别名，最终目标仍然是未导出的类型，应产生编译错误
class Point {
  public x: double = 0.0
  public y: double = 0.0
}
type Helper = Point
export type Exported = Helper  // FAIL: Helper 指向未导出的 Point
