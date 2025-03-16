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
  expected_imports_base: False
---*/

export abstract class A {    
    protected x: int;
    public abstract return_x(): int;
};

export abstract class B extends A {
    protected y: int;
    public abstract return_y(): int
};

export abstract class C extends B {
    protected z: int;
    public abstract return_z(): int
};

export class D extends C {
    public override return_x(): int
    {
        return this.x;
    }
    public override return_y(): int
    {
        return this.y;
    }
    public override return_z(): int
    {
        return this.z;
    }
}

function main(): void {
    undefined  // #BP
    console.log("After evaluate");
    undefined;  // #BP
}
