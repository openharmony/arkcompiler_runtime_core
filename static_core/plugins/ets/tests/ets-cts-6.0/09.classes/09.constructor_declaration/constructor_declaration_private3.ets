/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    A constructor is used in the creation of an object that is an instance of a class.
    Private constructor.
---*/

class A {
    public x: int;

    private constructor fromInt(p: int) {
        this.x = p;
    }

    static create(): A {
        return new A.fromInt(5)
    }
}

function main() {
    arktest.assertEQ(A.create().x, 5)
}
