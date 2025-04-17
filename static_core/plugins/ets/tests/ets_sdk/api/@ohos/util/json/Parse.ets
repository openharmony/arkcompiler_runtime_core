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

import {json} from "@ohos.util.json";

function main() {
    testStringifyParseBigInt()
    testStringifyParseOptOne()
    testStringifyParseOptZero()
    testStringifyParseBigIntWithReviver()
    testParseNoBigInt()
    testParseNoBigIntParseAsBigint()
    testParseNoBigIntAlwaysParseAsBigint()
}

export class User {}

class UserWithFields extends User {
    id: BigInt = new BigInt()
    g: Boolean = false
    name: String = ""
}

class SuperUser extends UserWithFields {
    users: UserWithFields[] = []
}

function testStringifyParseBigInt(): void {
    let classType = Type.of(new SuperUser() as Object)
    let str = "{\"id\":10,\"g\":true,\"name\":\"Name\",\"users\":[{\"id\":12,\"g\":true,\"name\":\"Name1\"},{\"id\":13,\"g\":false,\"name\":\"Name2\"}]}";
    let x = json.stringify(json.parse<SuperUser>(str, classType, undefined, {bigIntMode: json.BigIntMode.ALWAYS_PARSE_AS_BIGINT} as json.ParseOptions) as SuperUser)
    assertEQ(x, str)
}

function testStringifyParseOptOne(): void {
    let classType = Type.of(new SuperUser() as Object)
    let str = "{\"id\":10,\"g\":true,\"name\":\"Name\",\"users\":[{\"id\":12,\"g\":true,\"name\":\"Name1\"},{\"id\":13,\"g\":false,\"name\":\"Name2\"}]}";
    expectThrow(() => {json.parse<SuperUser>(str, classType,  undefined, {bigIntMode: json.BigIntMode.PARSE_AS_BIGINT} as json.ParseOptions)})
}

function testStringifyParseOptZero(): void {
    let classType = Type.of(new SuperUser() as Object)
    let str = "{\"id\":10,\"g\":true,\"name\":\"Name\",\"users\":[{\"id\":12,\"g\":true,\"name\":\"Name1\"},{\"id\":13,\"g\":false,\"name\":\"Name2\"}]}";
    expectThrow(() => {json.parse<SuperUser>(str, classType,  undefined, {bigIntMode: json.BigIntMode.DEFAULT} as json.ParseOptions)})
}

function testStringifyParseBigIntWithReviver(): void {
    let classType = Type.of(new SuperUser() as Object)
    let reviver = (k: string, v: NullishType) => {
        if (k === "id") {
            return new BigInt(1)
        }
        return v
    }
    let str = "{\"id\":10,\"g\":true,\"name\":\"Name\",\"users\":[{\"id\":12,\"g\":true,\"name\":\"Name1\"},{\"id\":13,\"g\":false,\"name\":\"Name2\"}]}";
    let expected_str = "{\"id\":1,\"g\":true,\"name\":\"Name\",\"users\":[{\"id\":1,\"g\":true,\"name\":\"Name1\"},{\"id\":1,\"g\":false,\"name\":\"Name2\"}]}";
    let x = json.stringify(json.parse<SuperUser>(str, classType, reviver, {bigIntMode: json.BigIntMode.ALWAYS_PARSE_AS_BIGINT} as json.ParseOptions) as SuperUser)
    assertEQ(x, expected_str)
}


class NumbersTest extends User {
    val: Number = 1
    str: String = ""
    bul: Boolean = false
}

function testParseNoBigInt() {
    let classType = Type.of(new NumbersTest())
    let str = "{\"val\":123444444421,\"str\":\"true\",\"bul\":false}"
    let x = json.stringify(json.parse<NumbersTest>(str, classType, undefined, {bigIntMode: json.BigIntMode.DEFAULT} as json.ParseOptions) as NumbersTest)
    assertEQ(x, str)
}

function testParseNoBigIntParseAsBigint() {
    let classType = Type.of(new NumbersTest())
    let str = "{\"val\":123444444421,\"str\":\"true\",\"bul\":false}"
    let x = json.stringify(json.parse<NumbersTest>(str, classType, undefined, {bigIntMode: json.BigIntMode.PARSE_AS_BIGINT} as json.ParseOptions) as NumbersTest)
    assertEQ(x, str)
}

function testParseNoBigIntAlwaysParseAsBigint() {
    let classType = Type.of(new NumbersTest())
    let str = "{\"val\":123444444421,\"str\":\"true\",\"bul\":false}"
    expectThrow(() => {json.parse<NumbersTest>(str, classType, undefined, {bigIntMode: json.BigIntMode.ALWAYS_PARSE_AS_BIGINT} as json.ParseOptions) as NumbersTest})
}
