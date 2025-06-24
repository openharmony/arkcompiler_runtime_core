/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

 let mainId = CoroutineExtras.getWorkerId();

function main() {
    let job1 = EAWorker.postToMain<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        assertEQ(runId, mainId);
        return runId;
    });

    let job2 = EAWorker.postToMain<int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        assertEQ(runId, mainId);
        return runId;
    });

    let job3 = launch<int, () => int>(():int => {
        let runId = CoroutineExtras.getWorkerId();
        assertNE(runId, mainId);
        return runId;
    });

    let runId1 = job1.Await();
    let runId2 = job2.Await();
    let runId3 = job3.Await();

    assertEQ(runId1, runId2);
    assertNE(runId1, runId3);
}


