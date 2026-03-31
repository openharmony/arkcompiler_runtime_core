/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

function regularFunctionThatThrows(): void {
    throw new Error("Regular error")
}

function testRegularMethodStackTrace(): void {
    try {
        regularFunctionThatThrows()
    } catch (e) {
        let stack = (e as Error).stack!
        arktest.assertNE(stack.indexOf(".ets:"), -1, "Regular method stack should contain file path with '.ets:' suffix")
        arktest.assertNE(stack.indexOf("regularFunctionThatThrows"), -1, "Stack should contain 'regularFunctionThatThrows'")
    }
}

class NativeMethodTestClass {
    public static native testNativeMethod(): boolean

    public static callNativeMethod(): void {
        let result = NativeMethodTestClass.testNativeMethod()
    }
}

function testNativeMethodStackTrace(): void {
    try {
        NativeMethodTestClass.callNativeMethod()
    } catch (e) {
        let stack = (e as Error).stack!
        arktest.assertNE(stack.indexOf("(Native Method)"), -1, "Native method stack should contain '(Native Method)'")
        arktest.assertNE(stack.indexOf("testNativeMethod"), -1, "Stack should contain 'testNativeMethod'")
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("StackTrace Native Method Format Test")

    suite.addTest("Test regular method stack trace format", testRegularMethodStackTrace)
    suite.addTest("Test native method shows (Native Method)", testNativeMethodStackTrace)

    return suite.run()
}
