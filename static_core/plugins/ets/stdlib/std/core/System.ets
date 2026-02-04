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
 * @file Defines the System for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Exits the current program or script with the specified exit code.
 * @param { int } code The exit code of the program. Typically, 0 indicates normal exit,
 *     non-zero values indicate abnormal exit.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function exit(code: int): void

/**
 * Prints current call stack information to standard error output.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export native function printStackTrace(): void

/**
 * Gets current call stack information and returns it as a string array.
 * @returns { FixedArray<String> } A string array containing call stack information,
 *     where each string represents one line of the call stack.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export function stackTraceLines(): FixedArray<String> {
    return new FixedArray<String>(0);
}
