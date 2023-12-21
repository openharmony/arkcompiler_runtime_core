/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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


'use strict';

function a1() {
  'use concurrent';
  return 1;
}
a1();

function a2() {
  'use concurrent';
}
a2();

async function a3() {
  'use concurrent';
  await 1;
  return 1;
}
a3();

async function a4() {
  'use concurrent';
  await 1;
}
a4();

async function* asyncGenerator() {
  let i = 0;
  const count = 3;
  const time = 1000;
  while (i < count) {
    await new Promise(resolve => setTimeout(resolve, time));
    yield i++;
  }
}

(async () => {
  for await (const value of asyncGenerator()) {
    console.log(value);
  }
})();