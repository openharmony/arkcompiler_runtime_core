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
desc: Using class inheritance and interface implementation to verify the correctness of the type system, but not all interface methods are implemented.
tags: [compile-only, negative]
---*/

class Animal {
    move(): string {
        return "Moving!";
    }
}

interface Swimmable {
    swim(): string;
}

class Fish extends Animal implements Swimmable {
    move(): string {
        return "Moving!";
    }
}