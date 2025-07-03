/*
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

function getEmptyTuple(): [] {
    return [];
}

function getTestPrimitiveTuple(): [boolean, char, byte, short, int, long, float, double] {
    return [true, c'x', 127, 300, 500, 600, 100.0, 400.0];
}

function getCharTuple(): [char, char, char, char, char] {
    return [c'H', c'e', c'l', c'l', c'o']
}

function getBooleanTuple(): [boolean, boolean, boolean, boolean, boolean] {
    return [true, false, true, false, true];
}

function getByteTuple(): [byte, byte, byte, byte, byte] {
    return [125, 126, 127, 1, 2];
}

function getShortTuple(): [short, short, short, short, short] {
    return [300, 350, 200, 100, 50];
}

function getIntTuple(): [int, int, int, int, int] {
    return [300, 350, 200, 100, 50];
}

function getLongTuple(): [long, long, long, long, long] {
    return [30000, 20000, 10000, 5000, 100000];
}

function getFloatTuple(): [float, float, float, float, float] {
    return [3.14, 2.71, 1.61, 0.59, 10.0];
}

function getDoubleTuple(): [double, double, double, double, double] {
    return [3.145, 2.716, 1.611, 0.594, 10.08];
}
