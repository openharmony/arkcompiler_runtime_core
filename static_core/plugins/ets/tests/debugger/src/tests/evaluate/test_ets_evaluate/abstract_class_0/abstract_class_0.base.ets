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
evaluation:
  expected_imports_base: True
---*/

export abstract class A {
    constructor(state_rhs: int)
    {
        this.state = state_rhs;
    }

    public return_state(): int
    {
        return this.state;
    }

    private state: int;
};

export class B extends A {
    constructor(state_rhs: int, val: int)
    {
        super(state_rhs);
        this.b_x = val;
    }

    private b_x: int;
};

export class C extends B {
    constructor(state_rhs: int, val: int)
    {
        super(state_rhs, val);
        this.c_x = val;
    }

    private c_x: int;
};

function main(): void {
    let variable: A = new B(1, 2);
    undefined;  // #BP{boolean}
    console.log("After evaluate");
    undefined;  // #BP
}
