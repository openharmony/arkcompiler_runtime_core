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
desc: Bigint multiplication is associative
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("BigIntAssociativity");

  suite.addTest("Associativity with small values", () => {
    let a = 2n;
    let b = 3n;
    let c = 5n;
    arktest.assertEQ((a * b) * c, a * (b * c));
    arktest.assertEQ((a * b) * c, 30n);
  });

  suite.addTest("Associativity with negative values", () => {
    let a = -2n;
    let b = 3n;
    let c = -5n;
    arktest.assertEQ((a * b) * c, a * (b * c));
    arktest.assertEQ((a * b) * c, 30n);
  });

  suite.addTest("Associativity with large values", () => {
    let a = 1000000n;
    let b = 2000000n;
    let c = 3000000n;
    arktest.assertEQ((a * b) * c, a * (b * c));
    arktest.assertEQ((a * b) * c, 6000000000000000000n);
  });

  suite.addTest("Associativity with zero", () => {
    let a = 0n;
    let b = 999n;
    let c = 888n;
    arktest.assertEQ((a * b) * c, a * (b * c));
    arktest.assertEQ((a * b) * c, 0n);
  });

  suite.addTest("Associativity with one", () => {
    let a = 1n;
    let b = 37n;
    let c = 1n;
    arktest.assertEQ((a * b) * c, a * (b * c));
    arktest.assertEQ((a * b) * c, 37n);
  });

  return suite.run();
}
