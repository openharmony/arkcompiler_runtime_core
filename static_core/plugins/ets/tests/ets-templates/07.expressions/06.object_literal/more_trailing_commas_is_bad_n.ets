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
desc: The comma at the end of the composite class is ignored but more produces error
tags: [negative, compile-only]
expected_out: |-
    Syntax error ESY0016: Unexpected token.
    Semantic error ESE0065: type Person has no property named *ERROR_LITERAL*
    Syntax error ESY0228: Unexpected token, expected '('.
    Syntax error ESY0230: Expected '{', got '}'.
    Syntax error ESY0194: Parameter declaration should have an explicit type annotation.
    Syntax error ESY0229: Unexpected token, expected ',' or ')'.
    Syntax error ESY0112: Unexpected token, expected an identifier.
    Syntax error ESY0230: Expected '}', got 'end of stream'.
---*/

class Person {
    name: String = ""
    age: int
}

function main(): void {
    let p: Person = { name: "Alice", age: 25,, }
}
