/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: Interface field declaration
---*/

interface BaseColors {
  RED: int;
  GREEN: int;
  BLUE: int;
}

interface ExtendedColors extends BaseColors {
  YELLOW: int;
}

interface BaseColorsWithUndefined {
  RED: int;
  GREEN ?: int;
  BLUE: int;
}

interface ExtendedColorsWithUndefined extends BaseColorsWithUndefined {
  YELLOW ?: int;
}

function main() {
  let i: ExtendedColors = { RED: 1, GREEN: 2, BLUE: 3, YELLOW: 4}
  let j: ExtendedColorsWithUndefined = { RED: 1, BLUE: 3}
  arktest.assertEQ(i.RED, 1)
  arktest.assertEQ(i.GREEN, 2)
  arktest.assertEQ(i.BLUE, 3)
  arktest.assertEQ(i.YELLOW, 4)
  arktest.assertEQ(j.RED, 1)
  arktest.assertEQ(j.GREEN, undefined)
  arktest.assertEQ(j.BLUE, 3)
  arktest.assertEQ(j.YELLOW, undefined)
}
