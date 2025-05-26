/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

let etsConcatArray1: Array<Number> = new Array<Number>(1, 2, 3, 4)
let etsConcatArray2: Array<Number> = new Array<Number>(5, 6, 7, 8)

function arrayConcat(arg1: Array<number>, arg2: Array<number>) {
  // Array.concat takes ConcatArray as parameter, which is a interface
  return arg1.concat(arg2);
}