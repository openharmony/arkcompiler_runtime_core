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
    public static array: double[] = [1.0, 2.0, 3.0, 4.0, 5.0];

    public static ChangeStaticArray() {
        ArrayClass.array[2] = 22.0;
        ArrayClass.array[4] = 44.0;
    }

    public static CheckStaticArray(): boolean {
        return (
            ArrayClass.array[0] as int == 1.0 &&
            ArrayClass.array[1] as int == 2.0 &&
            ArrayClass.array[2] as int == 22.0 &&
            ArrayClass.array[3] as int == 33.0 &&
            ArrayClass.array[4] as int == 44.0
        )
    }
}

function GetArray(): double[] {
    let a : double[] = [1.0, 2.0, 3.0, 4.0, 5.0]
    return a
}

function GetEscompatArray(): Array<double> {
    let a : Array<double> = new Array<double>(5)
    a[0] = 1.0
    a[1] = 2.0
    a[2] = 3.0
    a[3] = 4.0
    a[4] = 5.0
    return a
}

function CheckArray(a: double[]): boolean {
    return a[0] as int == 1 && a[1] as int == 2 && a[2] as int == 30 && a[3] as int == 40 && a[4] as int == 50
}

function GetSpecialArray(): double[] {
	let min: double = Number.MIN_VALUE
	let max: double = Number.MAX_VALUE
	let minus: double = -Number.MAX_VALUE
    let a : double[] = [min, max, minus, 0, 0, 0]
    return a
}

function CheckEscompatArray(a: Array<double>): boolean {
    return a[0] as int == 1 &&
           a[1] as int == 2 &&
           a[2] as int == 30 &&
           a[3] as int == 40 &&
           a[4] as int == 50
}
