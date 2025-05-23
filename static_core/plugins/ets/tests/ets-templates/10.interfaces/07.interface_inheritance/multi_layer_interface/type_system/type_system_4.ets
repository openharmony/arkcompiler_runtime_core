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
desc: A class implements multiple interfaces to verify the correctness of the type system.
---*/

interface Runnable {
    run(): string;
}

interface Jumpable {
    jump(): string;
}

class Athlete implements Runnable, Jumpable {
    run(): string {
        return "Running!";
    }

    jump(): string {
        return "Jumping!";
    }
}

function main(): void {
    const athlete: Athlete = new Athlete();
    assertEQ(athlete.run(), "Running!");
    assertEQ(athlete.jump(), "Jumping!");
}