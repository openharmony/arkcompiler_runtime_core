/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

package std.time;

export final class Chrono {
    /// Count of nanoseconds in 1 millisecond
    public static readonly NS_PER_MS: long = 1000000;

    // now() in nanoseconds
    public static native nanoNow(): long;

    // now() in milliseconds
    public static milliNow(): number {
        return Chrono.nanoNow() / 1000000;
    }

    /**
     * @returns cpu time in nanoseconds
     */
    public static native getCpuTime(): long;
}
