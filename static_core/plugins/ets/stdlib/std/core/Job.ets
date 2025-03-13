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

package std.core;

enum JobStatus { RUNNING = 0, FINISHED = 1, FAILED = 2 }

/**
 * Object of Job class is used to represent a task
 * that was launched using std.concurrency.mlaunch function.
 */
export final class Job<T> {
    private constructor() {}

    /**
     * This method causes current coroutine to wait for completion of
     * the job, and returns awaited object in case of successful completion.
     *
     * @returns unwrapped value.
     */
    public native Await(): T

    private val: T
    private mutex: Mutex = new Mutex()
    private event: Event = new Event()
    private status : int = JobStatus.RUNNING
}
