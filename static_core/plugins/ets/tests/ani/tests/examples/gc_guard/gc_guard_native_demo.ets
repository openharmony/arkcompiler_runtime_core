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

/*---
file_name: ani_test_example_gc_guard_native
---*/

// The native library's ANI_Constructor binds this method.  This is the same
// ETS -> loadLibrary -> native-function route used by runtime-owned clients.
class GcGuardNativeDemo {
    static { loadLibrary("gc_guard_native_demo") }

    static native native_func(): void;
}

export function callNativeFunc(): void {
    GcGuardNativeDemo.native_func();
}
