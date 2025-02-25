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

class ArrayClass {
    public static array: short[] = [1, 2, 3, 4, 5];

    public static ChangeStaticArray() {
        ArrayClass.array[2] = 222;
        ArrayClass.array[4] = 444;
    }

    public static CheckStaticArray(): boolean {
        return (
            ArrayClass.array[0] == 1 &&
            ArrayClass.array[1] == 2 &&
            ArrayClass.array[2] == 222 &&
            ArrayClass.array[3] == 333 &&
            ArrayClass.array[4] == 444
        )
    }
}

function GetArray(): short[] {
    let a : short[] = [1, 2, 3, 4, 5]
    return a
}

function GetEscompatArray(): Array<short> {
    let a : Array<short> = new Array<short>(5)
    a[0] = 1 as short
    a[1] = 2 as short
    a[2] = 3 as short
    a[3] = 4 as short
    a[4] = 5 as short
    return a
}

function CheckArray(a: short[]): boolean {
    return a[0] == 1 && a[1] == 2 && a[2] == 30 && a[3] == 40 && a[4] == 50
}

function GetSpecialArray(): short[] {
	let min: short = -32768
	let max: short = 32767
    let a : short[] = [min, max, 0, 0, 0]
    return a
}

function CheckEscompatArray(a: Array<short>): boolean {
    return a[0] == 1 &&
           a[1] == 2 &&
           a[2] == 30 &&
           a[3] == 40 &&
           a[4] == 50
}
