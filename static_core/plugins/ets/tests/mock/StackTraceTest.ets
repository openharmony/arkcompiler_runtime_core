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

class StackTraceTest {
    public static invokeException(): void {
        try {
            StackTraceTest.nestedFunc1();
        } catch (e: Exception)  {
            console.println(e.toString());
        } catch (e) {
            assertTrue(false, "Unexpected catch clause");
        }
    }

    public static invokeUnhandledException(): void throws {
        StackTraceTest.nestedFunc1();
    }

    static nestedFunc1(): void throws {
        StackTraceTest.nestedFunc2();
    }

    static nestedFunc2(): void throws {
        StackTraceTest.throwing();
    }

    static throwing(): void throws {
        throw new Exception("Test message");
    }
}
