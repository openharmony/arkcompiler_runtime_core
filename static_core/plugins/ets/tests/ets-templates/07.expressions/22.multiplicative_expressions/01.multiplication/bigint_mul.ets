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
desc: Multiplication of bigint numbers
---*/

function main(): int {
  let suite = new arktest.ArkTestsuite("BigIntMultiplication");

  suite.addTest("Zero * Zero", () => {
    arktest.assertEQ(0n * 0n, 0n);
  });

  suite.addTest("Zero * Large", () => {
    arktest.assertEQ(0n * 123456789n, 0n);
  });

  suite.addTest("One * One", () => {
    arktest.assertEQ(1n * 1n, 1n);
  });

  suite.addTest("Identity", () => {
    arktest.assertEQ(35n * 1n, 35n);
  });

  suite.addTest("Positive * Positive", () => {
    arktest.assertEQ(7n * 8n, 56n);
  });

  suite.addTest("Negative * Negative", () => {
    arktest.assertEQ(-7n * -8n, 56n);
  });

  suite.addTest("Positive * Negative", () => {
    arktest.assertEQ(7n * -8n, -56n);
  });

  suite.addTest("Negative * Positive", () => {
    arktest.assertEQ(-7n * 8n, -56n);
  });

  suite.addTest("Int32 boundary", () => {
    arktest.assertEQ(2147483647n * 2n, 4294967294n);
  });

  suite.addTest("Int64 boundary", () => {
    arktest.assertEQ(9223372036854775807n * 2n, 18446744073709551614n);
  });

  suite.addTest("Large numbers", () => {
    arktest.assertEQ(1000000000000000000n * 1000000000000000000n, 1000000000000000000000000000000000000n);
  });

  suite.addTest("Commutativity", () => {
    arktest.assertEQ(5n * 7n, 7n * 5n);
  });

  return suite.run();
}
