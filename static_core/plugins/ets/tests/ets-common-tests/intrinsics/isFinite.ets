/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

function main(): int {
    if (Float.isFinite(Float.NaN)) {
        return 1;
    }
    if (Float.isFinite(Float.POSITIVE_INFINITY)) {
        return 2;
    }
    if (Float.isFinite(Float.NEGATIVE_INFINITY)) {
        return 3;
    }
    if (!Float.isFinite(Float.MAX_VALUE)) {
        return 4;
    }
    if (!Float.isFinite(Float.MIN_VALUE)) {
        return 5;
    }
    if (!Float.isFinite(1.2345 as float)) {
        return 6;
    }
    if (Double.isFinite(Double.NaN)) {
        return 11;
    }
    if (Double.isFinite(Double.POSITIVE_INFINITY)) {
        return 12;
    }
    if (Double.isFinite(Double.NEGATIVE_INFINITY)) {
        return 13;
    }
    if (!Double.isFinite(Double.MAX_VALUE)) {
        return 14;
    }
    if (!Double.isFinite(Double.MIN_VALUE)) {
        return 15;
    }
    if (!Double.isFinite(1.2345)) {
        return 16;
    }
    return 0;
}
