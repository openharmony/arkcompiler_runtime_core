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

class NullPtrTrick {
    private p: string
    private leak() { return this.p }
    public constructor() { this.p = this.leak() }
    public static nullptr(): string { return (new NullPtrTrick()).p }
}

let npeCount: int = 0;

function countNPE(error: Error) {
    if (error instanceof NullPointerError) {
        ++npeCount;
    } else {
        throw error;
    }
}

function main(): void {
    // 1
    try {
        "a".compareTo(NullPtrTrick.nullptr());
    } catch (e) {
        countNPE(e);
    }
    // 2
    try {
        "a".startsWith(NullPtrTrick.nullptr())
    } catch (e) {
        countNPE(e);
    }
    // 3
    try {
        "a".endsWith(NullPtrTrick.nullptr())
    } catch (e) {
        countNPE(e);
    }
    // 4
    try {
        // concat2
        "a".concat(NullPtrTrick.nullptr())
    } catch (e) {
        countNPE(e);
    }
    // 5
    try {
        // concat3
        "a".concat("a", NullPtrTrick.nullptr())
    } catch (e) {
        countNPE(e);
    }
    // 6
    try {
        // concat4
        "a".concat("a", "a", NullPtrTrick.nullptr())
    } catch (e) {
        countNPE(e);
    }
    // 7
    try {
        "a".toLocaleLowerCase(NullPtrTrick.nullptr())
    } catch (e) {
        countNPE(e);
    }
    // 8
    try {
        "a".toLocaleUpperCase(NullPtrTrick.nullptr())
    } catch (e) {
        countNPE(e);
    }
    // 9
    try {
        "a".indexOf(NullPtrTrick.nullptr())
    } catch (e) {
        countNPE(e);
    }
    // 10
    try {
        "a".lastIndexOf(NullPtrTrick.nullptr())
    } catch (e) {
        countNPE(e);
    }
    arktest.assertEQ(npeCount, 10);
}
