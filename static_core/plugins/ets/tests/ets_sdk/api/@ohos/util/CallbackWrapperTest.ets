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
import {util} from "@ohos.util";

function main() {
    const suite = new arktest.ArkTestsuite("callbackWrapper tests");
    suite.addTest("testCallbackWrapperResolvesString", testUtilCallbackWrapper001);
    suite.addTest("testCallbackWrapperResolvesInt", testUtilCallbackWrapper002);
    suite.addTest("testCallbackWrapperResolvesError", testUtilCallbackWrapper003);
    suite.addTest("testCallbackWrapperWithArguments", testUtilCallbackWrapper004);
    return suite.run()
}

function testUtilCallbackWrapper001() {
    const promiseFn = async (): Promise<string> => {
        return 'value';
    };
    let cb = util.callbackWrapper(promiseFn);
    cb.unsafeCall((err: Error | null, ret: string) => {
        arktest.assertEQ(err, null);
        arktest.assertEQ(ret, 'value');
    })
}

function testUtilCallbackWrapper002() {
    const promiseFn = async (): Promise<int> => {
        return 42;
    };
    let cb = util.callbackWrapper(promiseFn);
    cb.unsafeCall((err: Error | null, ret: int) => {
        arktest.assertEQ(err, null);
        arktest.assertEQ(ret, 42);
    })
}

function testUtilCallbackWrapper003() {
    const promiseFn = async (): Promise<Error> => {
        let err = new Error('value');
        return err;
    };
    let cb = util.callbackWrapper(promiseFn);
    cb.unsafeCall((err: Error | null, ret: Object) => {
        arktest.assertEQ(err!.message, 'value');
        arktest.assertEQ(ret, undefined);
    })
}

function testUtilCallbackWrapper004() {
    const promiseFn = async (a: number, b: number): Promise<number> => {
        return a + b;
    };
    let cb = util.callbackWrapper(promiseFn);
    cb.unsafeCall(1.0, 2.0, (err: Error | null, ret: number) => {
        arktest.assertEQ(err, null);
        arktest.assertEQ(ret, 3.0);
    })
}
