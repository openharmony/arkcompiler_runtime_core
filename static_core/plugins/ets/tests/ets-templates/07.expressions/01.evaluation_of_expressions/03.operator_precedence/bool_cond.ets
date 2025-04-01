/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: conditional && and || operators on boolean values
---*/

let res = "";

function t1(): boolean { res += "1"; return true; }
function t2(): boolean { res += "2"; return false; }
function t3(): boolean { res += "3"; return true; }
function t4(): boolean { res += "4"; return false; }

function main(): int {
    res = ""
    if (t1() && t2() || t3()) {} else return 1;  // TFT
    if (res != "123") return 1;

    res = ""
    if (t1() || t2() && t3()) {} else return 1;  // TFT
    if (res != "1") return 1;

    res = ""
    if (t2() || t3() && t4()) return 1;  // FTF
    if (res != "234") return 1;

    res = ""
    if (t2() && t3() || t4()) return 1;  // FTF
    if (res != "24") return 1;

    res = ""
    if (!t1() && t2() || t3()) {} else return 1;  // FFT
    if (res != "13") return 1;

    res = ""
    if (!t1() || t2() && t3()) return 1;  // FFT
    if (res != "12") return 1;

    res = ""
    if (!t2() || t3() && t4()) {} else return 1;  // TTF
    if (res != "2") return 1;

    res = ""
    if (!t2() && t3() || t4()) {} else return 1;  // TTF
    if (res != "23") return 1;

    res = ""
    if (t1() && !t2() || t3()) {} else return 1;  //TTT
    if (res != "12") return 1;

    res = ""
    if (t1() || !t2() && t3()) {} else return 1; // TTT
    if (res != "1") return 1;

    res = ""
    if (t2() || !t3() && t4()) return 1;  // FFF
    if (res != "23") return 1;

    res = ""
    if (t2() && !t3() || t4()) return 1;  // FFF
    if (res != "24") return 1;

    return 0;
}
