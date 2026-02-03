/*
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
import taskpool from "@ohos.taskpool";
import { Callback } from "./@ohos.base";
import taskpool2 from "./bridge1";

export function testFunc2(a: Callback<string>): Callback<taskpool.Task> {
    return (data: taskpool.Task) => {};
}

export declare class TestClass {
    v1: taskpool.Task;
    v2: taskpool2.Task;
    v3: Callback<string>;
}
