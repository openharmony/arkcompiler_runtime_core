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
desc: >-
    Simple names that refer to top-level constants are not constant expressions in tuple
    indices.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0200: Only integer type allowed for element access on tuples."
---*/

const idx: int = 1

function main() {
    let tup: [number, string] = [1.0, 'a']
    let value = tup[idx]
}
