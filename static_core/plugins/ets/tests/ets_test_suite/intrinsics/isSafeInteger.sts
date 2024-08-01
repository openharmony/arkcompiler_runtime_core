/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

class IsIntegerFloatTest {
  safeInteger: float;
  unsafeInteger: float;
}

class IsIntegerDoubleTest {
  safeInteger: double;
  unsafeInteger: double;
}

function main(): int {
    let ft = new IsIntegerFloatTest();
    ft.safeInteger = 1.0;
    ft.unsafeInteger = 1.0 as float + Float.MAX_SAFE_INTEGER;

    if (Float.isSafeInteger(Float.NaN) != false) return 1;
    if (Float.isSafeInteger(Float.POSITIVE_INFINITY) != false) return 2;
    if (Float.isSafeInteger(Float.NEGATIVE_INFINITY) != false) return 3;
    if (Float.isSafeInteger(Float.MAX_SAFE_INTEGER as float) != true) return 4;
    if (Float.isSafeInteger(ft.safeInteger) != true) return 5;
    if (Float.isSafeInteger(ft.unsafeInteger) != false) return 6;

    let dt = new IsIntegerDoubleTest();
    dt.safeInteger = 1.0;
    dt.unsafeInteger = 1.0 as double + Double.MAX_SAFE_INTEGER;

    if (Double.isSafeInteger(Double.NaN) != false) return 21;
    if (Double.isSafeInteger(Double.POSITIVE_INFINITY) != false) return 22;
    if (Double.isSafeInteger(Double.NEGATIVE_INFINITY) != false) return 23;
    if (Double.isSafeInteger(Double.MAX_SAFE_INTEGER as double) != true) return 24;
    if (Double.isSafeInteger(dt.safeInteger) != true) return 25;
    if (Double.isSafeInteger(dt.unsafeInteger) != false) return 26;

    return 0;
}
