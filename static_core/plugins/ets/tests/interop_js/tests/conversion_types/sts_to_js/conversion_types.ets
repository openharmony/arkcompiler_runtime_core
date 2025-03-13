/**
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

package conversion_types.test;

const buffer = new ArrayBuffer(1);
const dataView = new DataView(buffer);
dataView.setUint8(0, 255)

const globStsString: String = new String("Hello");
const globStsNumber: Number = new Number(42 as number);
const globStsInt: Int = new Int(42);
const globStsByte: Byte = new Byte(126 as byte);
const globStsShort: Short = new Short(4242 as short);
const globStsLong: Long = new Long(42 as long);
const globStsFloat: Float = new Float(42.42 as float);
const globStsDouble: Double = new Double(42.42 as double);
const globStsChar: Char = new Char('a');
const globStsBoolean: Boolean = new Boolean(true);
const globStsBigInt: BigInt = new BigInt(42);

class DeferentIntTypes {
    systemHexadecimal = 16;
    systemBinary = 2;
    int: int = 1;
    number: number = 1;
    bigInt: BigInt = 1n;
    float: float = 1.1;
    intString: string = '1';
    long: long = 1;
    double: double = 1;
    char: char = 1;
    byte = dataView.getUint8(0);
}

class DeferentStringTypes {
    char: char = 'a';
    string: string = "hello";
    literal: "one" | "two" = "one";
}

class NullAndUndefinedTypes {
    etsNull: null = null;
    etsUndefined: undefined = undefined;
}
