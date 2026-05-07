/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Explicit type argument new Pos<number>(1) must be used to fix the code.
---*/

class Pos<T extends number> {
    constructor (x: T) {}
}

function main() {
    arktest.assertTrue(new Pos<number>(1) instanceof Pos, "expect instance of Pos - 1")
    arktest.assertTrue(new Pos<number>(new Number(1)) instanceof Pos, "expect instance of Pos - new Number(1)")
    arktest.assertTrue(new Pos<double>(1.0) instanceof Pos, "expect instance of Pos - 1.0")
}
