/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
desc: 7.5.3 Object Literal of Record Type
tags: []
---*/

class PersonInfo {
  name: string = ""
  age: number = 0
}

let J: PersonInfo = {
  name: "John",
  age: 20,
}

let A: PersonInfo = {
  name: "Alice",
  age: 25,
}

let C: PersonInfo = {
  name: "Chloe",
  age: 18,
}

function main() {
  let map: Record<String, PersonInfo> = {
    "John": J,
    "Alice": A,
    "Chloe": C,
  };

  assertEQ( map.size, 3)
  assertEQ( map.get("John"), J)
  assertEQ( map.get("Alice"), A)
  assertEQ( map.get("Chloe"), C)
}
