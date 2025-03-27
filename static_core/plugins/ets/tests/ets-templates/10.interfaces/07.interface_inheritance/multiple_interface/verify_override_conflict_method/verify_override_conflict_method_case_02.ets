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
desc: Verify that a child class must override a default method when there is a conflict between parent and child interfaces from different modules.
files:
- ./parentModule.ets
- ./childModule.ets
---*/

import { Parent } from "./parentModule.ets";
import { Child } from "./childModule.ets";

class ParentImpl implements Parent {
    greet(): string {
        return "Hello from Parent (default)";
    }
}

class ChildImpl implements Child {
    greet(): string {
        return "Hello from Child (new implementation)";
    }
}

function main(): void {
    const child = new ChildImpl();
    assertEQ(child.greet(), "Hello from Child (new implementation)");
}