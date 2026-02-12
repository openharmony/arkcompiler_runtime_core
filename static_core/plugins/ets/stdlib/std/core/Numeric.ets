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
 * @file Defines the Numeric for ArkTS
 * @kit ArkTS
 */


package std.core;

/**
 * The abstract interface for numeric types, which can be implemented by any type representing a numeric value.
 *
 * @extends Object
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export abstract class Numeric extends Object {

    /**
     * Converts the value of the object to byte type and returns it.
     *
     * @returns {byte} The byte type value of the object.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract toByte(): byte;

    /**
     * Converts the value of the object to int type and returns it.
     *
     * @returns {int} The int type value of the object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract toInt(): int;

    /**
     * Converts the value of the object to short type and returns it.
     *
     * @returns {short} The short type value of the object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract toShort(): short;

    /**
     * Converts the value of the object to long type and returns it.
     *
     * @returns {long} The long type value of the object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract toLong(): long;

    /**
     * Converts the value of the object to float type and returns it.
     *
     * @returns {float} The float type value of the object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract toFloat(): float;

    /**
     * Converts the value of the object to double type and returns it.
     *
     * @returns {double} The double type value of the object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public abstract toDouble(): double;
}

/**
 * The abstract base class for all integer types.
 *
 * @extends Numeric
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export abstract class Integral extends Numeric {}

/**
 * The abstract base class for all floating-point types.
 *
 * @extends Numeric
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export abstract class Floating extends Numeric {}
