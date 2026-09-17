/**
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

// CMC GC does not collect active TL(thread-local)-regions, their size is 256 KB each
// Allocations < 256 KB stay in the TL-region and survive full GC
// To guarantee GC collects the object, allocate > 256 KB so the TL-region
// fills up, moves to recentFullRegionList_ and becomes eligible for GC
let savedRef: FixedArray<long> | null = null;

function allocLargeAndReturnFreeHeapSize(): long {
    const ALLOC_SIZE: int = 262145; // 256 KB + 1 Byte > 1 region (256 KB)
    savedRef = new FixedArray<long>(ALLOC_SIZE);
    return GC.getFreeHeapSize();
}

function main(): void {
    // Free allocations done during runtime creation (i.e. application RuntimeLinker)
    GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
    let freeSizeInStart = GC.getFreeHeapSize();

    let freeSizeAfterNew = allocLargeAndReturnFreeHeapSize();

    // Drop reference so GC can collect
    savedRef = null;
    GC.waitForFinishGC(GC.startGC(GC.Cause.FULL));
    let freeSizeAfterGc = GC.getFreeHeapSize();

    arktest.assertLT(0, freeSizeInStart);
    arktest.assertLE(freeSizeAfterNew, freeSizeInStart, 'After allocation free memory should not increase');
    arktest.assertLE(freeSizeAfterNew, freeSizeAfterGc, 'After GC free memory should not decrease');
    arktest.assertLE(freeSizeAfterGc, freeSizeInStart);
}
