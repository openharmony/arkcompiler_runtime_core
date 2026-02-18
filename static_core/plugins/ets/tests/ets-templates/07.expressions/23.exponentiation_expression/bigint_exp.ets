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
desc: Bigint exponentiation - both operands must be bigint
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("BigIntExponentiation");

  suite.addTest("Exponent is 0", () => {
    arktest.assertEQ(2n ** 0n, 1n);
    arktest.assertEQ(100n ** 0n, 1n);
    arktest.assertEQ(0n ** 0n, 1n);
  });

  suite.addTest("Exponent is 1", () => {
    arktest.assertEQ(2n ** 1n, 2n);
    arktest.assertEQ(42n ** 1n, 42n);
  });

  suite.addTest("Base is 0", () => {
    arktest.assertEQ(0n ** 1n, 0n);
    arktest.assertEQ(0n ** 5n, 0n);
  });

  suite.addTest("Base is 1", () => {
    arktest.assertEQ(1n ** 100n, 1n);
  });

  suite.addTest("Powers of 2", () => {
    arktest.assertEQ(2n ** 2n, 4n);
    arktest.assertEQ(2n ** 3n, 8n);
    arktest.assertEQ(2n ** 10n, 1024n);
  });

  suite.addTest("Small operations", () => {
    arktest.assertEQ(3n ** 2n, 9n);
    arktest.assertEQ(5n ** 3n, 125n);
    arktest.assertEQ(10n ** 2n, 100n);
  });

  suite.addTest("Negative base even exponent", () => {
    arktest.assertEQ((-2n) ** 2n, 4n);
    arktest.assertEQ((-3n) ** 2n, 9n);
  });

  suite.addTest("Negative base odd exponent", () => {
    arktest.assertEQ((-2n) ** 3n, -8n);
    arktest.assertEQ((-3n) ** 3n, -27n);
  });

  suite.addTest("Large exponent", () => {
    arktest.assertEQ(2n ** 20n, 1048576n);
    arktest.assertEQ(2n ** 50n, 1125899906842624n);
  });

  suite.addTest("Result exceeds int64", () => {
    arktest.assertEQ(2n ** 64n, 18446744073709551616n);
  });

  return suite.run();
}
