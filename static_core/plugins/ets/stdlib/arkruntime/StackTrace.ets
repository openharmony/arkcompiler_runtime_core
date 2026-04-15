/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
/**
 * @file Defines the StackTrace for ArkTS
 * @kit ArkTS
 */

package arkruntime;

// synchronize with mirror class in runtime
/**
 * synchronize with mirror class in runtime
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 */
@annotation.InternalAPI
export class StackTraceElement {
    private static readonly NATIVE_METHOD_LINE_NUMBER: int = -2;

    private className: String = ""
    private methodName: String = ""
    private sourceFileName: String = ""
    private lineNumber: int
    private colNumber: int = 0

    private constructor () {}

    /**
     * Formats the stack trace element information into a string.
     *
     * @returns { String } data in formatted string.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public override toString(): String {
        return "\tat " + this.className + "." + this.methodName +
             // NOTE(@sandee, #24492): Extraction of column number is not implemented
             ((this.lineNumber !== StackTraceElement.NATIVE_METHOD_LINE_NUMBER) ?
             " (" + this.sourceFileName + ":" + this.lineNumber + ":" + this.colNumber + ")" : " (Native Method)")
    }
}

/**
 * Provides functionality to obtain the current call stack trace.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 */
@annotation.InternalAPI
export class StackTrace {
    /**
    * Method provides stack of methods at the place of a call
    *
    * @returns { FixedArray<StackTraceElement> } stack trace
    * @static
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    */
    static native provisionStackTrace(): FixedArray<StackTraceElement>
}
