/*
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

/*---
desc: OutOfMemoryError is thrown by an assignment to an array component of
    reference type, a method call expression, or a prefix / postfix
    increment / decrement operator as a result of boxing conversion.
ark_options: ['--heap-size-limit=33554432', '--init-heap-size-limit=33554432']
timeout: 60
---*/

const ARRAY_SIZE = 1_000_000;

function main(): int {
  try {
    let v: Object[] = new Object[ARRAY_SIZE];

    for (let i: int = 0; i < ARRAY_SIZE; i++) {
        let w = new Double[ARRAY_SIZE];

        for (let j: int = 0; j < ARRAY_SIZE; j++) {
          w[j] = new Double();
        }

        v[i] = w as Object;
    }
    return 1;
  } catch (e: OutOfMemoryError) {
    return 0;
  } catch (e) {
    return 1;
  }
  return 1;
}
