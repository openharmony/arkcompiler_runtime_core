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
desc: "TEST-UNIT-09-0006 GAP-09-0006 GRC-09-0006 CI-09-02-002-01: Subclass can access protected members of parent"
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE0293: Property privateValue is not visible here.
---*/

class Parent {
    protected protectedValue: number = 100
    private privateValue: number = 200
}

class Child extends Parent {
    accessProtected(): number {
        return this.protectedValue  // OK: can access protected
    }

    accessPrivate(): number {
        return this.privateValue  // Should fail: cannot access private
    }
}

function main(): void {
    let child = new Child()
    console.log(child.accessProtected())
}
