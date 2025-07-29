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

let str: String = 'abcΣΨΩ0123456789!';
let bytes: byte[] = [97, 98, 99, -93, -88, -87, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 33]

function equals(val: byte[], exp: byte[], offset: int): boolean {
    if (val.length != (exp.length - offset)) {
        return false;
	}
    for (let i: int = 0; i < val.length; ++i) {
        if (val[i] != exp[i + offset]) {
            return false;
		}
    }
    return true;
}

function printArray(a: byte[], offset: int, name: String): void {
    console.println(name + ' length: ' + (a.length - offset))
    console.print(name + ' data:  [ ')
    for (let i: int = 0; i < a.length; ++i) {
        console.print(a[i + offset] + ' ');
    }
    console.println(']');
}

function main(): int {
    let len: int = str.getLength()
    for (let i: int = 0; i < len; ++i) {
        let arr: byte[] = str.getBytes(i, len);
        if (!equals(arr, bytes, i)) {
            console.println('at: ' + i);
            printArray(arr, 0, 'actual  ')
            printArray(bytes, i, 'expected')
            return 1;
        }
    }
    return 0;
}
