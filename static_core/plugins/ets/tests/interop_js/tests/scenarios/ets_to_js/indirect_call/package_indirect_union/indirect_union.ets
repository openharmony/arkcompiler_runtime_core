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

package indirect_call_test_type_union

function GCJSRuntimeCleanup(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let full_gc_id = GC.startGC(GC.Cause.FULL);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        assertTrue(false, "Unexpected exception during GC");
    }
}

function indirectCallArgTypeUnion(arg: number | string): number {
    if (typeof arg  == 'number') {
        return 0;
    } else {
        return 1;
    }
}

function indirectCallReturnTypeUnion(arg: number): (number | string) {
    if (arg == 0) {
        return 0;
    } else {
        return 'foo';
    }
}
