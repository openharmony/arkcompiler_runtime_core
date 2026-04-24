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

export namespace attach {

export enum DebugSessionAttachErrorCode {
    OK = 0,
    ALREADY_ATTACHED,
    NOT_ALLOWED,
    INVALID,
}

export native function attachDebugSession(): int

export native function countEventsForMethod(methodName: string): int

// Debug events are not recorded for methods continuing execution in compiled code
export native function isAotEnabled(): boolean

export native function attachViaAni(): int

export native function throwErrorViaAni(): void

export function attachViaReflection(): int {
    const m = Class.current().getStaticMethod('attachImpl')!
    return m.invoke() as int
}

export function attachImpl(): int {
    return attach.attachDebugSession()
}

}

// Deliberately put functions into a separate namespace in order to test that
// classes loaded after debug attach won't be linked with AOTed methods
export namespace fns {

export function sum(lhs: int, rhs: int): int {
    return lhs + rhs
}

export function sub(lhs: int, rhs: int): int {
    return lhs - rhs
}

export function mul(lhs: int, rhs: int): int {
    return lhs * rhs
}

}

export class Event {
    constructor() {
        this.mutex_ = new concurrency.stackless.AsyncMutex();
        this.flagCV_ = new concurrency.stackless.AsyncCondVar();
        this.flag_ = false;
    }

    public async wait(): Promise<undefined> {
        await this.mutex_.lock();
        if (!this.flag_) {
            await this.flagCV_.wait(this.mutex_);
        }
        this.mutex_.unlock();
    }

    public async setAll(): Promise<undefined> {
        await this.mutex_.lock();
        this.flagCV_.notifyAll(this.mutex_);
        this.flag_ = true;
        this.mutex_.unlock();
    }

    private mutex_: concurrency.stackless.AsyncMutex;
    private flagCV_: concurrency.stackless.AsyncCondVar;
    private flag_: boolean;
}
