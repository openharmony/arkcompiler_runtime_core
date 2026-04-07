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

package std.dfx;

/**
 * Provides functionality to obtain the current call stack trace.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 */
export class StackTraceHelper {
    /**
     * Method provides main thread stack of methods at the place of a call
     *
     * @returns { String } stack trace
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    static native getMainThreadStackTrace(): String
}
