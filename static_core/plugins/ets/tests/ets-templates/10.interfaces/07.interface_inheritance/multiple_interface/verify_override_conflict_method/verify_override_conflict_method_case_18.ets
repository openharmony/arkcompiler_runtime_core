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
desc: Test method parameter override in interface inheritance while keeping return type consistent.
tags: [compile-only, negative]
---*/

interface Parent {
    greet(): string;
}

class ParentImpl implements Parent {
    greet(): string {
        return "Hello from Parent";
    }
}

interface Child extends Parent {
    greet(age: number, name: string): string;
}

class ChildImpl implements Child {
    greet(age: number, name: string): string {
        return `Hello, ${name}, age ${age}`;
    }
}