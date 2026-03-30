/**
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

package arkruntime;

export namespace annotation {
    /**
     * InternalAPI layer APIs should only be used within the platform code
     */
    @Retention("SOURCE")
    export @interface InternalAPI {}

    /**
     * BytecodeAPI layer APIs should not be used within the source code
     */
    @Retention("SOURCE")
    export @interface BytecodeAPI {}

    @annotation.BytecodeAPI @Retention("RUNTIME")
    export @interface Module { /* properties are not defined */ }

    @annotation.BytecodeAPI @Retention("RUNTIME")
    export @interface FunctionReference { /* properties are not defined */ }

    @annotation.BytecodeAPI @Retention("RUNTIME")
    export @interface Async { /* properties are not defined */ }

    /**
    * An annotation that marks that a functional
    * object is named function.
    * @typedef { namedFunctionObject }
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    @annotation.BytecodeAPI
    export @interface NamedFunctionObject {
        name: string
    }

    /**
    * An annotation that marks that a functional
    * object was lowered from an async function.
    * @typedef { AsyncFunctionObject }
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    @annotation.BytecodeAPI
    export @interface AsyncFunctionObject {}

    /**
    * Defines ModuleDeclaration annotation
    * Support import from .abc file.
    * @syscap SystemCapability.Utils.Lang
    * @FaAndStageModel
    * @since 24
    */
    export @interface ModuleDeclaration {
        /**
        * string with declaration text, which is used for typecheck when binary import is performed
        * @type { string }
        * @syscap SystemCapability.Utils.Lang
        * @FaAndStageModel
        * @since 24
        */
        declaration: string;
    }

    /**
     * Defines OptionalParametersAnnotation annotation
     * Mark function which contain optional param and rest param.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
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
