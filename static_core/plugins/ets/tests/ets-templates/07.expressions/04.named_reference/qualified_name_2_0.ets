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
desc: QualifiedName refers to the Special syntax form of Record Indexing Expression.
---*/

type Keys = 'key1' | 'key2' | 'key3'

let x: Record<Keys, number> = {
    'key1': 1,
    'key2': 2,
    'key3': 4,
}

function main() {
    let y = x['key2'] // y value is 2
    arktest.assertEQ(2, y)
}
