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
desc: Division of bigint numbers - rounds toward 0
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("BigIntDivision");

  suite.addTest("Division by one", () => {
    arktest.assertEQ(48n / 1n, 48n);
    arktest.assertEQ(-48n / 1n, -48n);
  });

  suite.addTest("Self division", () => {
    arktest.assertEQ(48n / 48n, 1n);
  });

  suite.addTest("Positive / Positive exact", () => {
    arktest.assertEQ(100n / 10n, 10n);
  });

  suite.addTest("Negative / Negative exact", () => {
    arktest.assertEQ(-100n / -10n, 10n);
  });

  suite.addTest("Positive / Negative exact", () => {
    arktest.assertEQ(100n / -10n, -10n);
  });

  suite.addTest("Negative / Positive exact", () => {
    arktest.assertEQ(-100n / 10n, -10n);
  });

  suite.addTest("Round toward 0 - positive result", () => {
    arktest.assertEQ(7n / 2n, 3n);
  });

  suite.addTest("Round toward 0 - negative result", () => {
    arktest.assertEQ(-7n / 2n, -3n);
  });

  suite.addTest("Zero dividend", () => {
    arktest.assertEQ(0n / 1n, 0n);
    arktest.assertEQ(0n / -48n, 0n);
  });

  suite.addTest("Large / Small", () => {
    arktest.assertEQ(1000000n / 3n, 333333n);
    arktest.assertEQ(-1000000n / 3n, -333333n);
  });

  suite.addTest("Int32 boundary", () => {
    arktest.assertEQ(2147483647n / 1n, 2147483647n);
    arktest.assertEQ(-2147483648n / 1n, -2147483648n);
    arktest.assertEQ(4294967294n / 2n, 2147483647n);
  });

  suite.addTest("Int64 boundary", () => {
    arktest.assertEQ(9223372036854775807n / 1n, 9223372036854775807n);
    arktest.assertEQ(-9223372036854775808n / 1n, -9223372036854775808n);
    arktest.assertEQ(18446744073709551614n / 2n, 9223372036854775807n);
  });

  suite.addTest("Very large division", () => {
    arktest.assertEQ(1000000000000000000000000000000000000n / 1000000000000000000n, 1000000000000000000n);
  });

  return suite.run();
}
