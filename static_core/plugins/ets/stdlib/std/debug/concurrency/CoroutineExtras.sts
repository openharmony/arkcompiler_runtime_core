/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

package std.debug.concurrency;

export final class CoroutineExtras {
    public static POLICY_DEFAULT = 0;
    public static POLICY_NON_MAIN = 1;

    // returns coroutine worker id, which is the same for coros which are running within the same worker thread
    public static native getWorkerId(): int;
    // sets default coroutine affinity
    public static native setSchedulingPolicy(policy: int): void;
    // returns coroutine id
    public static native getCoroutineId(): int;
}
