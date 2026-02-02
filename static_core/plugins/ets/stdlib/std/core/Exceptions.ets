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
 * @file Defines the Exceptions for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Represents an error thrown when data is expected but not provided.
 * 
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class NoDataError extends Error {
    // NOTE(kparshukov): #28318 move to std/core/Errors.ets
    constructor(message?: String, options?: ErrorOptions) {
        super("NoDataError", message, options)
    }
}


/**
 * Represents exception that is thrown when provided argument have value outside the allowable range
 * 
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class ArgumentOutOfRangeError extends Error {
    /**
     * Constructs an ArgumentOutOfRangeError instance.
     * @param { String } [ message ] The error message.
     * @param { ErrorOptions } [ options ] Error options, usually containing the error stack information.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("ArgumentOutOfRangeError", message, options)
    }
}

/**
 * Represents error that is thrown when a method has been invoked at an illegal or inappropriate time.
 * 
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class IllegalStateError extends Error {
    /**
     * Constructs an IllegalStateError instance.
     * @param { String } [ message ] The error message.
     * @param { ErrorOptions } [ options ] Error options, 
     * usually containing the error stack information.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("IllegalStateError", message, options)
    }
}

/**
 * Represents error that is thrown when the requested operation is not supported.
 * 
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class UnsupportedOperationError extends Error {
    /**
     * Constructs an UnsupportedOperationError instance.
     * @param { String } [ message ] The error message.
     * @param { ErrorOptions } [ options ] Error options, 
     * usually containing the error stack information.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("UnsupportedOperationError", message, options)
    }
}

/**
 * Represents an error thrown when a method is passed an illegal argument.
 * 
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class IllegalArgumentError extends Error {
    /**
     * Constructs an IllegalArgumentError instance.
     * @param { String } [ message ] The error message.
     * @param { ErrorOptions } [ options ] Error options, 
     * usually containing the error stack information.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("IllegalArgumentError", message, options)
    }
}

/**
 * Represents an error that occurs during runtime.
 * 
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class RuntimeError extends Error {
    /**
     * Constructs a RuntimeError instance.
     * @param { String } [ message ] The error message.
     * @param { ErrorOptions } [ options ] Error options, 
     * usually containing the error stack information.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("RuntimeError", message, options)
    }
}