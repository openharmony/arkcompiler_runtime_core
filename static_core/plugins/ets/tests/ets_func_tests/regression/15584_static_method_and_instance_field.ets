/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Issue 15584. two members of a class can have the same name if one is static while the other is not.
name: regression/15584_static_method_and_instance_field.ets
tags: []
---*/

class Base {
    static foo(): number { return -10 }
}

class Test extends Base {
    foo: number = 10;
}

function main(): int {
    assertEQ(Test.foo(), -10)
    assertEQ((new Test).foo, 10)
    return 0;
}
