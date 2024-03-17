/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

const TWENTY_TWO_HUNDREDTH = 0.22;
const ONE = 1;
const TWO = 2;
const THREE = 3;
const FOUR = 4;
const FIVE = 5;
const FIFTEEN = 15;
const PRECISION = 3;

import { Methods } from './lib';

function AssertEq<T>(a: T, b: T) {
  console.log(`AssertEq: '${a}' === '${b}'`);
  if (a !== b) {
    throw new Error(`AssertEq failed: '${a}' === '${b}'`);
  }
}

export function main() {
  testMethods();
}

function testMethods() {
  AssertEq(Methods.StaticSumDouble(THREE, TWENTY_TWO_HUNDREDTH).toFixed(PRECISION), (THREE + TWENTY_TWO_HUNDREDTH).toFixed(PRECISION));

  const o = new Methods();
  AssertEq(o.IsTrue(false), false);

  AssertEq(o.SumString('Hello from ', 'Panda!!!!'), 'Hello from Panda!!!!');
  AssertEq(o.SumIntArray([ONE, TWO, THREE, FOUR, FIVE]), FIFTEEN);
  AssertEq(o.OptionalString('Optional string'), 'Optional string');

  // NOTE(ivagin): enable when supported by runtime #12808
  // NOTE(vpukhov): optional methods produce overload sets
  if (false) {
    // AssertEq(o.OptionalString(undefined), undefined);
    // AssertEq(o.OptionalString(), undefined);
  }

  // NOTE(ivagin): enable when supported by interop #12808
  if (false) {
    AssertEq(o.SumIntVariadic(ONE, TWO, THREE, FOUR, FIVE).toFixed(PRECISION), (FIFTEEN).toFixed(PRECISION));
  }
}
