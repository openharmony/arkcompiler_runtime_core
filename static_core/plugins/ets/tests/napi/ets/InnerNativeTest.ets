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

class InnerNativeTest {
    static {
        loadLibrary("etsnapi_lib");
    }

    class Inner {

        class InnerInner {
            native static innerNative(): int;
        }

        native innerNative(): int;

        public getInnerNative(): int {
            let inner2: InnerInner = new InnerInner;
            return inner2.innerNative();
        }
    }

    public getInnerNative(): int {
        let inner: Inner = new Inner;
        return inner.innerNative();
    }

    public getInnerInnerNative(): int {
        let inner: Inner = new Inner;
        return inner.getInnerNative();
    }
}

function main(): void {
    let a: InnerNativeTest = new InnerNativeTest;
    if (a.getInnerNative() != 2) {
        throw new Exception("ERROR: error return value from ETSNAPI");
        }
    if (a.getInnerInnerNative() != 3) {
        throw new Exception("ERROR: error return value from ETSNAPI");
        }
}