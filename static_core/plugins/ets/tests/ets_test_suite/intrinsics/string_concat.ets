/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

let init: String[] = [ "", "Z", "perseverance", "Я", "длинношеее"]
let addend1: String[] = [ "", "Q", " credentials", "Ф", " распространение"]
let addend2: String[] = [ "", "R", " essentially", "Ж", " многочлен"]
let addend3: String[] = [ "", "W", " hilariously", "Ю", " обоерукий"]
let golden1: String[][] = [ [ "", "Q", " credentials", "Ф", " распространение"],
                            [ "Z", "ZQ", "Z  credentials", "ZФ", "Z распространение"],
                            [ "perseverance", "perseveranceZ", "perseverance credentials",
                              "perseveranceФ", "perseverance распространение"],
                            [ "Я", "ЯQ", "Я credentials", "ЯФ", "Я распространение"],
                            [ "длинношеее", "длинношеееQ", "длинношеее credentials",
                              "длинношеееФ", "длинношеее распространение"]];
let golden2: String[][] = [[ "", "QR", " credentials essentially", "ФЖ",
                             "длинношеее распространение"],
                           [ "Z", "ZQR", "Z credentials", "Z credentials essentially",
                             "ZФЖ", "Z распространение многочлен"],
                           [ "perseverance", "perseveranceQR", "perseverance credentials essentially",
                             "perseveranceФЖ", "perseverance распространение многочлен"],
                           [ "Я", "ЯQR", "Я credentials essentially", "ЯФЖ", "Я распространение многочлен"],
                           ["длинношеее", "длинношеееQR","длинношеее credentials essentially",
                            "длинношеееФЖ","длинношеее распространение многочлен"]]
                        

let golden3: String[][] = [["", "QRW", " credentials essentially hilariously",
                            "ФЖЮ", " распространение многочлен обоерукий"],
                           ["Z", "ZQRW", "Z credentials essentially hilariously",
                             "ZФЖЮ", "Z распространение многочлен обоерукий"],
                           ["perseverance", "perseveranceQRW", "perseverance credentials essentially hilariously",
                             "perseveranceФЖЮ","perseverance распространение многочлен обоерукий"],
                           ["Я", "ЯQRW","Я credentials essentially hilariously",
                            "ЯФЖЮ","Я распространение многочлен обоерукий"],
                           ["длинношеее", "длинношеееQRW","длинношеее credentials essentially hilariously",
                            "длинношеееФЖЮ","длинношеее распространение многочлен обоерукий"]]
                        
function test_concat2(): int {
    for (let i: int = 0; i < init.length; ++i) {
        for (let j: int = 0; i < addend1.length; ++i) {
            let str: String = init[i];
            let str1: String = addend1[i];
            str.concat(str1);
            if (!str.equals(golden1[i][j])) {
                console.println("concat2 failed at iteration: " + i + ", " + j);
                console.println("expected: " + golden1[i][j]);
                console.println("actual:   " + str);
                return 1;
            }
        }
    }
    return 0;
}

function test_concat3(): int {
    for (let i: int = 0; i < init.length; ++i) {
        for (let j: int = 0; i < addend1.length; ++i) {
            let str: String = init[i];
            let str1: String = addend1[i];
            let str2: String = addend2[i];
            str.concat(str1, str2);
            if (!str.equals(golden2[i][j])) {
                console.println("concat3 failed at iteration: " + i + ", " + j);
                console.println("expected: " + golden2[i][j]);
                console.println("actual:   " + str);
                return 2;
            }
        }
    }
    return 0;
}

function test_concat4(): int {
    for (let i: int = 0; i < init.length; ++i) {
        for (let j: int = 0; i < addend1.length; ++i) {
            let str: String = init[i];
            let str1: String = addend1[i];
            let str2: String = addend2[i];
            let str3: String = addend3[i];
            str.concat(str1, str2, str3);
            if (!str.equals(golden3[i][j])) {
                console.println("concat4 failed at iteration: " + i + ", " + j);
                console.println("expected: " + golden3[i][j]);
                console.println("actual:   " + str);
                return 4;
            }
        }
    }
    return 0;
}

function main(): int {
    return test_concat2() + test_concat3() + test_concat4();
}
