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

import { attach, fns } from './utils'

function attachViaReflection(): int {
    const m = Class.current().getStaticMethod('attachImpl')!
    return m.invoke() as int
}

export function attachImpl(): int {
    return attach.attachDebugSession()
}

function main() {
    loadLibrary('ets_debug_session_attach_lib')

    arktest.assertEQ(attachViaReflection(), attach.DebugSessionAttachErrorCode.OK)
    if (!attach.isAotEnabled()) {
        arktest.assertLT(0, attach.countEventsForMethod('attachViaReflection'))
    }

    fns.sum(1, 2)
    arktest.assertLT(0, attach.countEventsForMethod('sum'))
}
