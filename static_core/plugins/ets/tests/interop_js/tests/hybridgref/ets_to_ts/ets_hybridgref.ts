/**
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

let plusOne = (x: number)=> {
    return x + 1;
}

function main(): void {
    let etsVm = globalThis.gtest.etsVm;

    let etsSaveNativeGrefChar = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefChar');
    etsSaveNativeGrefChar();
    let valueChar = nativeGetRef();
    ASSERT_EQ(typeof valueChar, "string");
    ASSERT_EQ(valueChar, "A");

    let etsSaveNativeGrefByte = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefByte');
    etsSaveNativeGrefByte();
    let valueByte = nativeGetRef();
    ASSERT_EQ(typeof valueByte, "number");
    ASSERT_EQ(valueByte, 0x12);

    let etsSaveNativeGrefShort = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefShort');
    etsSaveNativeGrefShort();
    let valueShort = nativeGetRef();
    ASSERT_EQ(typeof valueShort, "number");
    ASSERT_EQ(valueShort, 300);

    let etsSaveNativeGrefInt = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefInt');
    etsSaveNativeGrefInt();
    let valueInt = nativeGetRef();
    ASSERT_EQ(typeof valueInt, "number");
    ASSERT_EQ(valueInt, 123456);

    let etsSaveNativeGrefDouble = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefDouble');
    etsSaveNativeGrefDouble();
    let valueDouble = nativeGetRef();
    ASSERT_EQ(typeof valueDouble, "number");
    ASSERT_TRUE(Math.abs(valueDouble - 100.111) < 1e-10);

    let etsSaveNativeGrefFloat = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefFloat');
    etsSaveNativeGrefFloat();
    let valueFloat = nativeGetRef();
    ASSERT_EQ(typeof valueFloat, "number");
    ASSERT_TRUE(Math.abs(valueFloat - 1.5) < 1e-6);

    let etsSaveNativeGrefString = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefString');
    etsSaveNativeGrefString();
    let valueString = nativeGetRef();
    ASSERT_EQ(typeof valueString, "string");
    ASSERT_EQ(valueString, "hello world");

    let etsSaveNativeGrefNumberArray = etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefNumberArray");
    etsSaveNativeGrefNumberArray();
    let arr = nativeGetRef();
    ASSERT_TRUE(arr instanceof Array);
    ASSERT_EQ(arr.length, 5);
    ASSERT_EQ(arr[2], 3);

    let etsSaveNativeGrefObject = etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefObject");
    etsSaveNativeGrefObject();
    let obj = nativeGetRef();
    ASSERT_EQ(typeof obj, "object");
    ASSERT_EQ(obj.name, "ArkTS");
    ASSERT_EQ(obj.version, 1);

    let etsSaveNativeGrefMap = etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefMap");
    etsSaveNativeGrefMap();
    let map = nativeGetRef();
    ASSERT_TRUE(map instanceof Map);
    ASSERT_EQ(map.get("a"), 10);
    ASSERT_EQ(map.get("b"), 20);

    let etsSaveNativeGrefSet = etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefSet");
    etsSaveNativeGrefSet();
    let set = nativeGetRef();
    ASSERT_TRUE(set instanceof Set);
    ASSERT_TRUE(set.has("x"));
    ASSERT_TRUE(set.has("y"));

    let etsSaveNativeGrefObjectFromTS = etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefObjectFromTS");
    etsSaveNativeGrefObjectFromTS(plusOne);
    let plusOneGet = nativeGetRef();
    ASSERT_TRUE(plusOneGet === plusOne);
}

(globalThis as any).nativeGetRef();
main();
