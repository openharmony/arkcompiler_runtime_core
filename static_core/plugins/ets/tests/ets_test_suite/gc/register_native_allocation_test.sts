/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

function CreateWeakRefString(s: string): WeakRef<String>
{
    let obj = "Hello " + s;
    return new WeakRef<String>(obj);
}

function RegisterNativeAllocation(): void {
    let total: long = 0;
    let size: long = 16384;
    let exception_catched: boolean = false;
    try {
        GC.registerNativeAllocation(-size);
    } catch(error: NegativeArraySizeError) {
        exception_catched = true;
    }
    assertTrue(exception_catched, "Didn't catch the exception");
    exception_catched = false;
    try {
        GC.registerNativeFree(-size);
    } catch(error: NegativeArraySizeError) {
        exception_catched = true;
    }
    assertTrue(exception_catched, "Didn't catch the exception");

    for (let i = 0; i < 2048; i++) {
        GC.registerNativeAllocation(size);
        total += size;
    }
    
    while (total > 0) {
        GC.registerNativeFree(size);
        total -= size;
    }
}

function main(): int {
    let weak_ref_string = CreateWeakRefString("world!");

    try {
        // Calling RegisterNativeAllocation to trigger GC
        RegisterNativeAllocation();
    } catch(e) {
        assertTrue(false, "Unexpected exception");
    }

    assertTrue(weak_ref_string.deref() === undefined, "String referent must be collected after GC was triggered");
    return 0;
}