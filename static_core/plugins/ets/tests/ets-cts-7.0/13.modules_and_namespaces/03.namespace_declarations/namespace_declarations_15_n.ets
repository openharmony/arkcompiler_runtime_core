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
desc: ES2PANDA_FAIL - Hard keyword 'abstract' cannot be used as identifier
tags: [compile-only, negative]
expected_out: |-
    Syntax error ESY0316: Hard keyword 'abstract' cannot be used as identifier
    Syntax error ESY0224: Identifier expected, got 'abstract'.
    Syntax error ESY0227: Unexpected token 'abstract'.
---*/

namespace abstract {
    function foo(): string {
        return "Foo1"
    }
}

function main() {
    arktest.assertEQ(abstract.boo(), "Foo1")
}
