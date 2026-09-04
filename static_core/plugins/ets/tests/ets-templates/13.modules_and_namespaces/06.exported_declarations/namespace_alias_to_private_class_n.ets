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

// 命名空间中 export type 指向未导出的外部类型，应产生编译错误
class Point {
  public x: double = 0.0
  public y: double = 0.0
}
namespace Shapes {
    export type MyPoint = Point  // FAIL: Point is not exported
}
