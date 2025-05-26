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

function UseAndCheckScheduleGcTest(): void {
    let detected = false;
    try {
        GC.scheduleGcAfterNthAlloc(1, GC.YOUNG_CAUSE);
    } catch (e: UnsupportedOperationException) {
        detected = true;
    }
    catch (e) {}
    assertTrue(detected, "scheduleGcAfterNthAlloc must throw UnsupportedOperationException if you don't use it with needed flag");
}

function main(): int {
    UseAndCheckScheduleGcTest();
    return 0;
}
