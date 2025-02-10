/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

function simpleTest(): void throws {
    GC.scheduleGcAfterNthAlloc(2, GC.YOUNG_CAUSE);
    new Object();
    assertFalse(GC.isScheduledGCTriggered(), "GC was scheduled before 2nd allocation, current is 1st");
    new Object();
    assertTrue(GC.isScheduledGCTriggered(), "2 allocation already done, but GC has not been trigger");
}

function incorrectCounterTest(): void {
    let detected = false;
    try {
        GC.scheduleGcAfterNthAlloc(-1, GC.YOUNG_CAUSE);
    } catch (e: IllegalArgumentException) {
        detected = true;
    }
    catch (e) {}
    assertTrue(detected, "scheduleGcAfterNthAlloc must throw IllegalArgumentException for negative counter");
}

function main(): int {
    try {
        simpleTest();
    }
    catch (e) {
        assertTrue(false, "Simple test must not throw exceptions");
    }
    incorrectCounterTest();
    return 0;
}
