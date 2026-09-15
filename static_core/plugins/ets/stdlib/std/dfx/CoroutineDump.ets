/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

package std.dfx;

/**
 * Describes one coroutine captured in a diagnostic snapshot.
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 */
export final class CoroutineDumpInfo {
    private workerName_: string = ''
    private isMain_: boolean = false
    private coroutineName_: string = ''
    private status_: string = 'UNKNOWN'
    private type_: string = 'UNKNOWN'
    private waitReason_: string = 'NONE'
    private lockMode_: string = 'NONE'
    private readerCount_: int = 0
    private writerCount_: int = 0
    private callStack_: string = ''

    public get workerName(): string {
        return this.workerName_
    }

    public get isMain(): boolean {
        return this.isMain_
    }

    public get coroutineName(): string {
        return this.coroutineName_
    }

    public get status(): string {
        return this.status_
    }

    public get type(): string {
        return this.type_
    }

    public get waitReason(): string {
        return this.waitReason_
    }

    public get lockMode(): string {
        return this.lockMode_
    }

    public get readerCount(): int {
        return this.readerCount_
    }

    public get writerCount(): int {
        return this.writerCount_
    }

    public get callStack(): string {
        return this.callStack_
    }

    private constructor() {}
}

/**
 * Provides coroutine diagnostics intended for debugging and troubleshooting.
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 */
export final class CoroutineDump {
    private constructor() {}

    /**
     * Returns a complete best-effort snapshot of current stackful coroutines.
     * Lock wait fields are populated for Mutex and RWLock waits.
     *
     * @returns { CoroutineDumpInfo[] } Structured coroutine diagnostic records
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static dumpCoroutineInfo(): CoroutineDumpInfo[] {
        return CoroutineDump.dumpCoroutineInfoRaw()
    }

    private static native dumpCoroutineInfoRaw(): CoroutineDumpInfo[]
}
