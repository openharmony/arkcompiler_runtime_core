/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

let x: int = 5;

let z: float = 3.0;

while (x < 10) {
    x = x + 1;
}

for (z = 3.0; z < 10; z++) {
    console.println("Hello!");
}

if (x == z) {
    x += 4;
}

class A {
    k: float = 5.0;
    foo(): int {
        return 10;
    }
}

function main(): int {
    return 0;
}
