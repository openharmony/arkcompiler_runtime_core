/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: 9.6.10 Overriding by Instance Methods
name: 09.classes/09.06.method_declarations/09.06.10.overriding_by_instance_methods/override_optional_param_01
tags: []
---*/

class A {
    returnHello(op?: string): string {
        return "hello A ";
    }
}

class B extends A {
    returnHello(op?: string): string {
        return "hello B ";
    }
}

function main(): void {
    let b: A = new B()
    assertEQ( b.returnHello(), "hello B ")
}
