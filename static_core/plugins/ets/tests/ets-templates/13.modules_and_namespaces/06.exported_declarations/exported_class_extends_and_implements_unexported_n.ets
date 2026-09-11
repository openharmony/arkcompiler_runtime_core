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
    Semantic error ESE1236581: Type 'Base' used in export declaration(s) is not exported.
---*/

'use static'

// 导出类同时继承未导出类并实现未导出接口，应产生编译错误
class Base {
  public id: int = 0
}
export interface Identifiable {
    getId(): int
}
export class Entity extends Base implements Identifiable {
    getId(): int {
        return this.id
    }
}
