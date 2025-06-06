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
desc: Verify that a child interface overrides and calls parent implementations when three parent interfaces have the same default method.
---*/

interface Parent1 {
    greet(): string;
}

interface Parent2 {
    greet(): string;
}

interface Parent3 {
    greet(): string;
}

class Parent1Impl implements Parent1 {
    greet(): string {
        return "Hello from Parent1";
    }
}

class Parent2Impl implements Parent2 {
    greet(): string {
        return "Hello from Parent2";
    }
}

class Parent3Impl implements Parent3 {
    greet(): string {
        return "Hello from Parent3";
    }
}

interface Child extends Parent1, Parent2, Parent3 {
    greet(): string;
}

class ChildImpl implements Child {
    greet(): string {
        return `Child says: ${new Parent1Impl().greet()}, ${new Parent2Impl().greet()}, ${new Parent3Impl().greet()}`;
    }
}

function main(): void {
    const child = new ChildImpl();
    assertEQ(child.greet(), "Child says: Hello from Parent1, Hello from Parent2, Hello from Parent3");
}