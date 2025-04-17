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
    public static array: FixedArray<byte> = [1, 2, 3, 4, 5];

    public static ChangeStaticArray() {
        ArrayClass.array[2] = 22 as byte;
        ArrayClass.array[4] = 44 as byte;
    }

    public static CheckStaticArray(): boolean {
        return (
            ArrayClass.array[0] == 1 &&
            ArrayClass.array[1] == 2 &&
            ArrayClass.array[2] == 22 &&
            ArrayClass.array[3] == 33 &&
            ArrayClass.array[4] == 44
        )
    }
}

function GetArray(): FixedArray<byte> {
    let a : FixedArray<byte> = [1, 2, 3, 4, 5]
    return a
}

function CheckArray(a: FixedArray<byte>): boolean {
    return a[0] == 1 && a[1] == 2 && a[2] == 30 && a[3] == 40 && a[4] == 50
}

function GetSpecialArray(): FixedArray<byte> {
	let min: byte = -128
	let max: byte = 127
    let a : FixedArray<byte> = [min, max, 0, 0, 0]
    return a
}
