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
desc: Rejects an exported interface that extends a class instead of an interface.
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE0378: Interfaces cannot extend classes, only other interfaces.
---*/

'use static'

// 导出接口 extends 类而非接口，应产生编译错误
export class Base {
    public value: int = 0
}
export interface Extended extends Base {  // Interfaces may not extend classes.
    getValue(): int
}
