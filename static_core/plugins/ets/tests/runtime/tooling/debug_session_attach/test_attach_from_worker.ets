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

import { attach, fns, Event } from './utils'

function workerAttachImpl() {
    const s = attach.attachDebugSession()
    arktest.assertEQ(s, attach.DebugSessionAttachErrorCode.OK)
    fns.mul(2, 3)
}

async function main() {
    loadLibrary('ets_debug_session_attach_lib')

    const workerAttach = new EAWorker()
    const worker = new EAWorker()
    workerAttach.start()
    worker.start()

    const attachLock = new RWLock()
    const startEvent = new Event()

    const jobAttach = workerAttach.run<undefined>(async () => {
        const writeLock = attachLock.writeLock()
        writeLock.lock()
        await startEvent.setAll()

        workerAttachImpl()

        writeLock.unlock()
    })
    const job = worker.run<undefined>(async () => {
        await startEvent.wait()

        const readLock = attachLock.readLock()
        readLock.lock()
        fns.sub(2, 1)
        readLock.unlock()
    })

    await startEvent.wait()
    const readLock = attachLock.readLock()
    readLock.lock()
    fns.sum(2, 1)
    readLock.unlock()

    await jobAttach
    await job
    await workerAttach.join()
    await worker.join()

    if (!attach.isAotEnabled()) {
        arktest.assertLT(0, attach.countEventsForMethod('workerAttachImpl'))
    }
    arktest.assertLT(0, attach.countEventsForMethod('sum'))
    arktest.assertLT(0, attach.countEventsForMethod('sub'))
    arktest.assertLT(0, attach.countEventsForMethod('mul'))
}
