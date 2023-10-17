/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

export final class Coroutine {
    // suspends current coroutine and schedules the next ready one for execution
    public static native Schedule(): void
}

/**
 * TODO(konstanting, #I67QXC):
 * This class is to be moved to a separate internal package (e.g. "std.debug") that would not be a part of the
 * standard library. It would be used for testing, debugging, profiling, etc.
 * The exact way of doing this is TBD.
 * 
 * WARNING: do not use this class! it will be gone soon!
 */
export final class CoroutineExtras {
    // returns coroutine worker id, which is the same for coros which are running within the same worker thread
    public static native GetWorkerId(): int;
}

export const coroutine = new Coroutine();
