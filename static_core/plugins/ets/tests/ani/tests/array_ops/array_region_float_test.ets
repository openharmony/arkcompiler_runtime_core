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
    public static array: float[] = [1.0, 2.0, 3.0, 4.0, 5.0];

    public static ChangeStaticArray() {
        ArrayClass.array[2] = 22.0 as float;
        ArrayClass.array[4] = 44.0 as float;
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

function GetArray(): float[] {
    let a : float[] = [1.0, 2.0, 3.0, 4.0, 5.0]
    return a
}

function GetEscompatArray(): Array<float> {
    let a : Array<float> = new Array<float>(5)
    a[0] = 1.0 as float
    a[1] = 2.0 as float
    a[2] = 3.0 as float
    a[3] = 4.0 as float
    a[4] = 5.0 as float
    return a
}

function CheckArray(a: float[]): boolean {
    return a[0] as int == 1 && a[1] as int == 2 && a[2] as int == 30 && a[3] as int == 40 && a[4] as int == 50
}

function GetSpecialArray(): float[] {
	let min: float = new Float(1.40129846432481707e-45)
	let max: float = new Float(3.40282346638528860e+38)
	let minus: float = new Float(-3.40282346638528860e+38)
    let a : float[] = [min, max, minus, 0.0, 0.0, 0.0]
    return a
}

function CheckEscompatArray(a: Array<float>): boolean {
    return a[0] as int == 1 &&
           a[1] as int == 2 &&
           a[2] as int == 30 &&
           a[3] as int == 40 &&
           a[4] as int == 50
}
