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
desc: Class implements two interfaces(number type).
---*/

interface Flyable {
  fly(): number;
}

interface Swimmable {
  swim(): number;
}

class Bird implements Flyable, Swimmable {
  fly(): number {
    return 1;
  }   

  swim(): number {
    return 2;
  }
}

function main(): void {
  const tetsBird = new Bird();
  const result = tetsBird.fly();
  assertEQ(result, 1);
}