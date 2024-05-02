/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

const DUMP: boolean = false;
const HITS_EXPECTED: int = 21;
let golden: String = new String("abcΣΨΩ0123456789!");
let hits: int = 0;

let table1: char[][] = [
    [c'a', c'b', c'c', c'Σ', c'Ψ', c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'b', c'c', c'Σ', c'Ψ', c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'c', c'Σ', c'Ψ', c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'Σ', c'Ψ', c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'Ψ', c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'6', c'7', c'8', c'9', c'!'],
    [c'7', c'8', c'9', c'!'],
    [c'8', c'9', c'!'],
    [c'9', c'!'],
    [c'!'],
    []
];

function printArray(a: char[], name: String): void
{
    console.println(name + " length: " + a.length)
    console.print(name + " data:   [")
    for (let n: int = 0; n < a.length; ++n) {
        console.print(a[n]);
    }
    console.println("]");
}

function equals(lhs: char[], rhs: char[]): boolean
{
    if (lhs.length != rhs.length)
        return false;
    for (let i: int = 0; i < lhs.length; ++i) {
        if (lhs[i] != rhs[i])
            return false;
    }
    return true;
}

function main(): int {

    for (let i: int = 0; i <= golden.getLength(); ++i) {
        let arr: char[] = golden.getChars(i, golden.getLength());
        if (equals(arr, table1[i])) {
            ++hits;
        }
        if (DUMP) {
            printArray(table1[i], "expected");
            printArray(arr, "getChars");
            console.println();
        }
    }

    try {
        // end > length
        let arr: char[] = golden.getChars(1, golden.getLength() + 1);
    } catch (e: StringIndexOutOfBoundsError) {
        ++hits;
        if (DUMP) {
            console.println("end > length");
            console.println(e);
        }
    }

    try {
        // begin > end
        let arr: char[] = golden.getChars(2, 1);
    } catch (e: StringIndexOutOfBoundsError) {
        ++hits;
        if (DUMP) {
            console.println("begin > end");
            console.println(e);
        }
    }

    try {
        // begin < 0
        let arr: char[] = golden.getChars(-1, 1);
    } catch (e: StringIndexOutOfBoundsError) {
        ++hits;
        if (DUMP) {
            console.println("begin < 0");
            console.println(e);
        }
    }

    if (hits != HITS_EXPECTED) {
        if (DUMP) {
            console.println("hits: " + hits + " (expected " + HITS_EXPECTED + ")");
        }
        console.println("FAIL");
        return 1;
    }
    console.println("PASS");
    return 0;
}
