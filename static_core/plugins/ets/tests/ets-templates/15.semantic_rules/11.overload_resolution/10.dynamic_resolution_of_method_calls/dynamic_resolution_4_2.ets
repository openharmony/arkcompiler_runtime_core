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
desc: >-
    Separate classes implementing different subinterfaces use the default
    method from their own interface inheritance path.
---*/

interface DiamondBase {
    m(): string {
        return "Base";
    }
}

interface DiamondLeft extends DiamondBase {
    m(): string {
        return "Left";
    }
}

interface DiamondRight extends DiamondBase {
    m(): string {
        return "Right";
    }
}

class A implements DiamondLeft {
}

class B implements DiamondRight {
}

function main(): void {
    let bl: DiamondBase = new A();
    let br: DiamondBase = new B();

    arktest.assertEQ(bl.m(), "Left");
    arktest.assertEQ(br.m(), "Right");
}
