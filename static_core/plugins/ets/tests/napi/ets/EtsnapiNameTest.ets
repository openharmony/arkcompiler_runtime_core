/**
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

class EtsnapiNameTest {
    public native static objectArg(cb: Object | undefined): int;

    public native static methodOverloaded(): int;

    public native static methodOverloaded(i: int): int;

    public native static methodOverloaded(bool: boolean,  b: byte, c: char, d: double, f: float, i: int, l: long, s: short): int;

    public native static methodOverloaded(obj: Object | undefined, str: String, arr: FixedArray<double>): int;

    public run(): void throws {
        if (EtsnapiNameTest.objectArg(undefined) != 0) {
            throw new Exception("ERROR: failed to call objectArg");
        }
        if (EtsnapiNameTest.methodOverloaded() != 1) {
            throw new Exception("ERROR: failed to call methodOverloaded");
        }
        if (EtsnapiNameTest.methodOverloaded(2) != 2 ) {
            throw new Exception("ERROR: failed to call methodOverloaded");
        }
        if (EtsnapiNameTest.methodOverloaded(true, 1 as byte, c'\uD8FF', 2.2, 3.3 as float, 4, 5 as long, 6 as short) != 3 ) {
            throw new Exception("ERROR: failed to call methodOverloaded");
        }
        let arr: FixedArray<double> = new double[4];
        if (EtsnapiNameTest.methodOverloaded(undefined, "Str", arr) != 4) {
            throw new Exception("ERROR: failed to call methodOverloaded");
        }
    }

    public native 中文函数2不带参数(): String;

    public runClass(): void throws {
        if (this.中文函数2不带参数() != "passed") {
            throw new Exception("ERROR: failed to call 中文函数2不带参数");
        }
    }
}

native function nativeInGlobal(): int;

function main(): int throws {
    loadLibrary("etsnapi_lib");
    let a: EtsnapiNameTest = new EtsnapiNameTest;
    a.run();
    a.runClass();
    if (nativeInGlobal() != 1) {
        console.print("ERROR: failed to call nativeInGlobal");
        console.println();
        return 1;
    }
    return 0;
}
