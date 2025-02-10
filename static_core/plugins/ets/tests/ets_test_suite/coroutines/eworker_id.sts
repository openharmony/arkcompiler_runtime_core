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

let mainId = CoroutineExtras.getWorkerId();

function main() {
    let w = new EAWorker();
    let p1 = w.run(() => {
        let runId = CoroutineExtras.getWorkerId();
        assertNE(runId, mainId);
        return runId;
    });
    let p2 = w.run(() => {
        let runId = CoroutineExtras.getWorkerId();
        assertNE(runId, mainId);
        return runId;
    });
    w.stop();
    let runId1 = await p1;
    let runId2 = await p2;
    assertEQ(runId1, runId2);
}
