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

function main() {
    let mainId = CoroutineExtras.getWorkerId();

    CoroutineExtras.scaleWorkersPool(1);
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let workerId = await launch () => { return CoroutineExtras.getWorkerId(); }();
    CoroutineExtras.scaleWorkersPool(-1);
    assertNE(mainId, workerId);

    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
    workerId = await launch () => { return CoroutineExtras.getWorkerId(); }();
    assertEQ(mainId, workerId);
}
