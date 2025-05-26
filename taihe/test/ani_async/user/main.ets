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
import * as async_test from "@generated/async_test";
import * as System from "std/core/System";
loadLibrary("ani_async");

function testSyncAdd() {
    assertEQ(async_test.addSync(1, 2), 3, "addSync should correctly add 1 + 2");
    expectError(
        () => {async_test.addSync(0, 2)}, "some error happen in add impl");
}

function testAsyncAdd() {
    // Test async add with callback
    async_test.add(10, 20, (error: Error, data?: int) => {
        assertEQ(error.message, "", "Callback should not receive an error");
        assertEQ(data, 30, "add callback should receive 10 + 20 = 30");
    });

    // Test promise rejection when first parameter is 0
    expectError(
        () => {await async_test.add(0, 2)}, "some error happen in add impl");
}

function testIBase() {
    const ibase = async_test.getIBase();
    assertEQ(ibase.get(), "My IBase", "IBase.get() should return 'My IBase'");

    // Test async interface method
    ibase.setAsync("interface async set", (error: Error) => {
        assertEQ(
            error.message, "", "setAsync callback should not receive an error");

        let p_ibase = ibase.getPromise();
        assertEQ(
            await p_ibase, "interface async set",
            "getPromise should resolve with the value set in setAsync");
    });
}

function testStaticAsyncMethods() {
    // Test static async method with callback
    async_test.IBase.printAsync((error: Error) => {
        assertEQ(
            error.message, "",
            "printAsync callback should not receive an error");
    });

    // Test getIBaseAsync callback
    async_test.getIBaseAsync((error: Error, data?: async_test.IBase) => {
        assertEQ(
            error.message, "",
            "getIBaseAsync callback should not receive an error");
        assertNE(data, undefined, "data should not be undefined");
        if (data) {
            assertEQ(
                data.get(), "My IBase",
                "IBase from getIBaseAsync should return 'My IBase'");
        }
    });

    // Test promise-based IBase getter
    let r = await async_test.getIBasePromise();
    assertNE(
        r, undefined, "getIBasePromise should resolve with an IBase instance");
    if (r) {
        assertEQ(
            r.get(), "My IBase",
            "IBase from getIBasePromise should return 'My IBase'");
    }
}

function testStructMethods() {
    const testData: async_test.Data = {a: "abc", b: "efg", c: 3};

    // Test fromStruct with callback
    async_test.fromStruct(testData, (error: Error) => {
        assertEQ(
            error.message, "",
            "fromStruct callback should not receive an error");
    });

    async_test.fromStruct(testData).catch((error: NullishType) => {
        assertEQ(
            (error as Error).message, "",
            "fromStruct callback should not receive an error");
    });

    // Test toStruct with callback
    async_test.toStruct(
        "aaa", "bbb", 1, (error: Error, data?: async_test.Data) => {
            assertEQ(
                error.message, "",
                "toStruct callback should not receive an error");
            assertNE(data, undefined, "data should not be undefined");
            if (data) {
                assertEQ(data.a, "aaa", "Data.a should be 'aaa'");
                assertEQ(data.b, "bbb", "Data.b should be 'bbb'");
                assertEQ(data.c, 1, "Data.c should be 1");
            }
        });

    // Test toStruct exception when c is 0
    expectError(
        () => {await async_test.toStruct("aaa", "bbb", 0)},
        "some error happen in toStructSync_impl");

    let result = await async_test.toStruct("aaa", "bbb", 1);
    assertEQ(result.a, "aaa", "Data.a should be 'aaa'");
    assertEQ(result.b, "bbb", "Data.b should be 'bbb'");
    assertEQ(result.c, 1, "Data.c should be 1");
}

function main() {
    const suite = new ArkTestsuite("Async Test Suite");

    suite.addTest("Sync Add Tests", testSyncAdd);
    suite.addTest("Async Add Tests", testAsyncAdd);
    suite.addTest("IBase Tests", testIBase);
    suite.addTest("Static Async Methods Tests", testStaticAsyncMethods);
    suite.addTest("Struct Methods Tests", testStructMethods);

    System.exit(suite.run());
}
