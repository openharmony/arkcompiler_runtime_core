/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Switch statement
---*/

function f(i: int): int {
  switch (i) {
    case 0: i++;
    case 1: i++;
    default: i++;
  }
  return i
}

function main(): void {
  assertEQ(f(0), 3)
  assertEQ(f(1), 3)
  assertEQ(f(2), 3)
}
