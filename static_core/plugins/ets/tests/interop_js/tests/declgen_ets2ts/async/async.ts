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

const TEN = 10;

import { PromiseWrapper, AsyncIdentity } from './lib';

function AssertEq<T>(a: T, b: T) {
  console.log(`AssertEq: '${a}' === '${b}'`);
  if (a !== b) {
    throw new Error(`AssertEq failed: '${a}' === '${b}'`);
  }
}

export function main() {
  testAsync();
}

function testAsync() {
  // NOTE(ivagin): enable when supported by interop #12808
  if (false) {
    const promiseWrapper = new PromiseWrapper();
    promiseWrapper.promise_.then((v: string) => AssertEq(v, 'Panda string'));
  }
  AsyncIdentity(TEN).then((v) => AssertEq(v, TEN));
}
