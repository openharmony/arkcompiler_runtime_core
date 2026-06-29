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

/*---
   tags: [no-warmup]
---*/

// This scenario validates exact console output, so it uses a paired .expected file instead of arktest assertions.

async function main(): Promise<int> {
    const sharedLabel = 'MAIN_TASKPOOL_SHARED'

    console.count(sharedLabel)

    await taskpool.execute((): void => {
        console.count(sharedLabel)
    })

    console.count(sharedLabel)
    console.countReset(sharedLabel)
    CoroutineExtras.stopTaskpool()
    return 0
}
