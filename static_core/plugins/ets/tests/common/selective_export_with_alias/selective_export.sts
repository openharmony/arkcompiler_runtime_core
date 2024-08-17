/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

function test_func(): void {}

let msg = "hello";

class TestClass {
    test_member = "test_member";

    test_method(): void {}
}

class AliasClass {
    alias_class_method(): void {
        console.print("alias_class_method");
    }
}

type aliased = AliasClass;

export {
    test_func as func, msg,
    TestClass as tc,
    aliased
}
