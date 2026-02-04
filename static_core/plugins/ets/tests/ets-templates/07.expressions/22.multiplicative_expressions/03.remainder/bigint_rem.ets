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
desc: Remainder operation on bigint - satisfies (a/b) * b + (a%b) = a
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("BigIntRemainder");

  suite.addTest("Exact division - zero remainder", () => {
    arktest.assertEQ(10n % 5n, 0n);
    arktest.assertEQ(100n % 10n, 0n);
    arktest.assertEQ(-100n % 10n, 0n);
    arktest.assertEQ(100n % -10n, 0n);
  });

  suite.addTest("Remainder with one", () => {
    arktest.assertEQ(36n % 1n, 0n);
    arktest.assertEQ(-36n % 1n, 0n);
  });

  suite.addTest("Self remainder", () => {
    arktest.assertEQ(36n % 36n, 0n);
    arktest.assertEQ(-36n % -36n, 0n);
  });

  suite.addTest("Positive % Positive", () => {
    arktest.assertEQ(7n % 3n, 1n);
    arktest.assertEQ(17n % 5n, 2n);
  });

  suite.addTest("Negative % Positive", () => {
    arktest.assertEQ(-7n % 3n, -1n);
  });

  suite.addTest("Positive % Negative", () => {
    arktest.assertEQ(7n % -3n, 1n);
  });

  suite.addTest("Negative % Negative", () => {
    arktest.assertEQ(-7n % -3n, -1n);
  });

  suite.addTest("Zero dividend", () => {
    arktest.assertEQ(0n % 1n, 0n);
    arktest.assertEQ(0n % 999999n, 0n);
    arktest.assertEQ(0n % -36n, 0n);
  });

  suite.addTest("Dividend < Divisor", () => {
    arktest.assertEQ(3n % 10n, 3n);
    arktest.assertEQ(-3n % 10n, -3n);
  });

  suite.addTest("Identity: (a/b)*b + (a%b) = a", () => {
    let a = 17n;
    let b = 5n;
    arktest.assertEQ((a / b) * b + (a % b), a);

    a = -17n;
    b = 5n;
    arktest.assertEQ((a / b) * b + (a % b), a);

    a = 17n;
    b = -5n;
    arktest.assertEQ((a / b) * b + (a % b), a);

    a = -17n;
    b = -5n;
    arktest.assertEQ((a / b) * b + (a % b), a);
  });

  suite.addTest("Large numbers", () => {
    arktest.assertEQ(1000000n % 7n, 1n);
    arktest.assertEQ(9223372036854775807n % 10n, 7n);
  });

  suite.addTest("Very large bigint", () => {
    let a = 123456789012345678901234567890n;
    let b = 7n;
    arktest.assertEQ((a / b) * b + (a % b), a);
  });

  return suite.run();
}
