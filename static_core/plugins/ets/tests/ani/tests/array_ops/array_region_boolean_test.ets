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
    public static array: boolean[] = [true, true, true, true, true];

    public static ChangeStaticArray() {
        ArrayClass.array[2] = false;
        ArrayClass.array[4] = false;
    }

    public static CheckStaticArray(): boolean {
        return (
            ArrayClass.array[0] == true &&
            ArrayClass.array[1] == true &&
            ArrayClass.array[2] == false &&
            ArrayClass.array[3] == false &&
            ArrayClass.array[4] == false
        )
    }
}

function GetArray(): boolean[] {
    let a : boolean[] = [true, true, true, true, true]
    return a
}

function GetEscompatArray(): Array<boolean> {
    let a : Array<boolean> = new Array<boolean>(5)
    a[0] = true
    a[1] = true
    a[2] = true
    a[3] = true
    a[4] = true
    return a
}

function CheckArray(a: boolean[]): boolean {
    return a[0] == true && a[1] == true && a[2] == true && a[3] == false && a[4] == true
}

function CheckEscompatArray(a: Array<boolean>): boolean {
    return a[0] == true &&
           a[1] == true &&
           a[2] == true &&
           a[3] == false &&
           a[4] == true
}
