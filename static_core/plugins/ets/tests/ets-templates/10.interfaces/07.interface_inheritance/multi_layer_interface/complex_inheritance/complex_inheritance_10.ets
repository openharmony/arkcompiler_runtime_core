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
desc: Test method rewriting in multi-level class inheritance.
---*/

interface G {
    greet(): string {
        return "Hello from G";
    }
}

interface F extends G {
    
}

interface E extends F {
    
}

class A implements E {
    greet(): string {
        return "Hello from A";
    }
}

class B extends A {
    greet(): string {
        return "Hello from B";
    }
}

class C extends B {
    greet(): string {
        return "Hello from C";
    }
}

function main(): void {
    const c = new C();
    assertEQ(c.greet(), "Hello from C");
}