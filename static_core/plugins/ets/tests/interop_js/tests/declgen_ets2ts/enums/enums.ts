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

const ZERO = 0;
const ONE = 1;
const TWO = 2;
const ONE_HUNDRED = 100;
const NINETY_NINE = 99

import { Color, Letter/*, Name*/ } from './lib';

function AssertEq<T>(a: T, b: T) {
  console.log(`AssertEq: '${a}' === '${b}'`);
  if (a !== b) {
    throw new Error(`AssertEq failed: '${a}' === '${b}'`);
  }
}

export function main() {
  testClasses();
}

function testClasses() {
  AssertEq(Letter.A, ZERO);
  AssertEq(Letter.B, ONE);
  AssertEq(Letter.C, TWO);

  AssertEq(Color.Red, ZERO);
  AssertEq(Color.Green, -ONE_HUNDRED);
  AssertEq(Color.Blue, -NINETY_NINE);
  AssertEq(Color.White, ZERO);

  // NOTE(ivagin): enable when supported by declgen #12808
  // AssertEq(Name.Ivan, "Ivan");
  // AssertEq(Name.Li, "Li");
}
