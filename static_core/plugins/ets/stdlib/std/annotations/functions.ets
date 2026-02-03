/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
 * @file Defines the functions for ArkTS
 * @kit ArkTS
 */


package std.annotations;

/**
 * Defines functions
 * @namespace functions
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export namespace functions {
    /**
     * Defines OptionalParametersAnnotation annotation
     * Mark function which contain optional param and rest param.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    @Retention("RUNTIME")
    export @interface OptionalParametersAnnotation {
        /**
         * Minimum number of optional param and rest param.
         * @type { int }
         * @syscap SystemCapability.Utils.Lang
         * @FaAndStageModel
         * @since 24
         */
        minArgCount: int
    };
}