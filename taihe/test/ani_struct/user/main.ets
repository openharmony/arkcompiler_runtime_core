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
import * as StructTest from "@generated/struct_test";
import * as System from "std/core/System";

loadLibrary("ani_struct");

function main() {
    console.log("run main ... StructTest ...")

    let data: StructTest.Data = {a: "abc", b: "efg", c: 3};
    let res = StructTest.concatStruct(data)
    console.log("result is " + res)
    assertEQ(res, "abcefg3")

    let newStruct = StructTest.makeStruct("aaa", "bbb", 1)
    console.log(
        "new struct is " + newStruct.a + " " + newStruct.b + " " + newStruct.c)
    assertEQ(newStruct.a, "aaa")
    assertEQ(newStruct.b, "bbb")
    assertEQ(newStruct.c, 1)
}