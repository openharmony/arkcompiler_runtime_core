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

class FakeNativeTest {
    static {
        loadLibrary("etsnapi_lib");
    }

    public native static nativeFake(): void;

    // @CriticalNative
    public native static nativeFakeCritical(): void;

    public run(): void {
        let isNativeFakeCalled: boolean = true;

        try {
            FakeNativeTest.nativeFake();
        } catch (e: Error) {
            isNativeFakeCalled = false;
        }

        if (isNativeFakeCalled) {
            throw new Error("ERROR: nativeFake method was successfully called, but does not have a definition");
        }

        /* TODO: enable this call when annotations are ready
        let isNativeFakeCriticalCalled: boolean = true;

        try {
            nativeFakeCritical();
        } catch (e: Exception) {
            isNativeFakeCriticalCalled = false;
        }

        if (isNativeFakeCriticalCalled) {
            throw new Error("ERROR: nativeFakeCritical method was successfully called, but does not have a definition");
        }
        */
    }
}

function main(): void {
    let a: FakeNativeTest = new FakeNativeTest;
    a.run();
}
