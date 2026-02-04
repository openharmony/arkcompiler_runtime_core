/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
 * @file Defines the Box for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Represents a generic object wrapper class
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class Box<T> {
    private value: T|null;

    /**
     * Creates a new instance of Box with null value.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = null;
    }

    /**
     * Creates a new instance of Box with the specified initial value.
     *
     * @param { T } value - The initial value to be wrapped
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: T) {
        this.value = value;
    }

    /**
     * Creates a new Box instance with the specified initial value.
     *
     * @param { TT } value - The initial value to be wrapped
     * @returns {Box<TT>} A new Box instance containing the specified value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static box<TT>(value: TT): Box<TT> {
        return new Box<TT>(value);
    }

    /**
     * Gets the value wrapped in this Box.
     *
     * @returns { T } The value wrapped in this Box
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get(): T {
        return this.value as T;
    }

    /**
     * Sets the value wrapped in this Box.
     *
     * @param { T } value - The new value to set
     * @returns { T } The newly set value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set(value: T): T {
        return (this.value = value) as T;
    }
}

/**
 * Represents a wrapper class for boolean primitive values.
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class BooleanBox {
    private value: boolean;

    /**
     * Creates a new BooleanBox instance with an initial value of false.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = false;
    }

    /**
     * Creates a new BooleanBox instance with the specified initial value.
     *
     * @param { boolean } value - The initial boolean value to be wrapped
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: boolean) {
        this.value = value;
    }

    /**
     * Creates a new BooleanBox instance with the specified initial value.
     *
     * @param { boolean } value - The initial boolean value to be wrapped
     * @returns { BooleanBox } A new BooleanBox instance containing the specified boolean value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static box(value: boolean): BooleanBox {
        return new BooleanBox(value);
    }

    /**
     * Gets the boolean value wrapped in this BooleanBox.
     *
     * @returns { boolean } The boolean value wrapped in this BooleanBox
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get(): boolean {
        return this.value;
    }

    /**
     * Sets the boolean value wrapped in this BooleanBox.
     *
     * @param { boolean } value - The new boolean value to set
     * @returns { boolean } The newly set boolean value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set(value: boolean): boolean {
        return (this.value = value);
    }
}

/**
 * Represents a wrapper class for byte primitive values.
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class ByteBox {
    private value: byte;

    /**
     * Creates a new ByteBox instance with an initial value of 0.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = 0;
    }

    /**
     * Creates a new ByteBox instance with the specified initial value.
     *
     * @param { byte } value - The initial byte value to be wrapped
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: byte) {
        this.value = value;
    }

    /**
     * Creates a new ByteBox instance with the specified initial value.
     *
     * @param { byte } value - The initial byte value to be wrapped
     * @returns { ByteBox } A new ByteBox instance containing the specified byte value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static box(value: byte): ByteBox {
        return new ByteBox(value);
    }

    /**
     * Gets the byte value wrapped in this ByteBox.
     *
     * @returns { byte } The byte value wrapped in this ByteBox
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get(): byte {
        return this.value;
    }

    /**
     * Sets the byte value wrapped in this ByteBox.
     *
     * @param { byte } value - The new byte value to set
     * @returns { byte } The newly set byte value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set(value: byte): byte {
        return (this.value = value);
    }
}

/**
 * Represents a wrapper class for char primitive values.
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class CharBox {
    private value: char;


    /**
     * Creates a new CharBox instance with an initial value of null character (c'\0').
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = c'\0';
    }

    /**
     * Creates a new CharBox instance with the specified initial value.
     *
     * @param { char } value - The initial char value to be wrapped
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: char) {
        this.value = value;
    }

    /**
     * Creates a new CharBox instance with the specified initial value.
     *
     * @param { char } value - The initial char value to be wrapped
     * @returns { CharBox } A new CharBox instance containing the specified char value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static box(value: char): CharBox {
        return new CharBox(value);
    }

    /**
     * Gets the char value wrapped in this CharBox.
     *
     * @returns { char } The char value wrapped in this CharBox
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get(): char {
        return this.value;
    }

    /**
     * Sets the char value wrapped in this CharBox.
     *
     * @param { char } value - The new char value to set
     * @returns { char } The newly set char value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set(value: char): char {
        return (this.value = value);
    }
}

/**
 * Represents a wrapper class for short primitive values.
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class ShortBox {
    private value: short;

    /**
     * Creates a new ShortBox instance with an initial value of 0.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = 0;
    }

    /**
     * Creates a new ShortBox instance with the specified initial value.
     *
     * @param { short } value - The initial short value to be wrapped
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: short) {
        this.value = value;
    }

    /**
     * Creates a new ShortBox instance with the specified initial value.
     *
     * @param { short } value - The initial short value to be wrapped
     * @returns { ShortBox } A new ShortBox instance containing the specified short value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static box(value: short): ShortBox {
        return new ShortBox(value);
    }

    /**
     * Gets the short value wrapped in this ShortBox.
     *
     * @returns { short } The short value wrapped in this ShortBox
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get(): short {
        return this.value;
    }

    /**
     * Sets the short value wrapped in this ShortBox.
     *
     * @param { short } value - The new short value to set
     * @returns { short } The newly set short value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set(value: short): short {
        return (this.value = value);
    }
}

/**
 * Represents a wrapper class for int primitive values.
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class IntBox {
    private value: int;

    /**
     * Creates a new IntBox instance with an initial value of 0.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = 0;
    }

    /**
     * Creates a new IntBox instance with the specified initial value.
     *
     * @param { int } value - The initial int value to be wrapped
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: int) {
        this.value = value;
    }

    /**
     * Creates a new IntBox instance with the specified initial value.
     *
     * @param { int } value - The initial int value to be wrapped
     * @returns { IntBox } A new IntBox instance containing the specified int value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static box(value: int): IntBox {
        return new IntBox(value);
    }

    /**
     * Gets the int value wrapped in this IntBox.
     *
     * @returns { int } The int value wrapped in this IntBox
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get(): int {
        return this.value;
    }

    /**
     * Sets the int value wrapped in this IntBox.
     *
     * @param { int } value - The new int value to set
     * @returns { int } The newly set int value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set(value: int): int {
        return (this.value = value);
    }
}

/**
 * Represents a wrapper class for long primitive values.
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class LongBox {
    private value: long;
    /**
     * Creates a new LongBox instance with an initial value of 0.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = 0;
    }

    /**
     * Creates a new LongBox instance with the specified initial value.
     *
     * @param { long } value - The initial long value to be wrapped
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: long) {
        this.value = value;
    }

    /**
     * Creates a new LongBox instance with the specified initial value.
     *
     * @param { long } value - The initial long value to be wrapped
     * @returns { LongBox } A new LongBox instance containing the specified long value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static box(value: long): LongBox {
        return new LongBox(value);
    }

    /**
     * Gets the long value wrapped in this LongBox.
     *
     * @returns { long } The long value wrapped in this LongBox
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get(): long {
        return this.value;
    }

    /**
     * Sets the long value wrapped in this LongBox.
     *
     * @param { long } value - The new long value to set
     * @returns { long } The newly set long value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set(value: long): long {
        return (this.value = value);
    }
}

/**
 * Represents a wrapper class for float primitive values.
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class FloatBox {
    private value: float;

    /**
     * Creates a new FloatBox instance with an initial value of 0.0f.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = 0.0f;
    }

    /**
     * Creates a new FloatBox instance with the specified initial value.
     *
     * @param { float } value - The initial float value to be wrapped
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: float) {
        this.value = value;
    }

    /**
     * Creates a new FloatBox instance with the specified initial value.
     *
     * @param { float } value - The initial float value to be wrapped
     * @returns { FloatBox } A new FloatBox instance containing the specified float value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static box(value: float): FloatBox {
        return new FloatBox(value);
    }

    /**
     * Gets the float value wrapped in this FloatBox.
     *
     * @returns { float } The float value wrapped in this FloatBox
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get(): float {
        return this.value;
    }

    /**
     * Sets the float value wrapped in this FloatBox.
     *
     * @param { float } value - The new float value to set
     * @returns { float } The newly set float value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set(value: float): float {
        return (this.value = value);
    }
}

/**
 * Represents a wrapper class for double primitive values.
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export final class DoubleBox {
    private value: double;

    /**
     * Creates a new DoubleBox instance with an initial value of 0.0.
     *
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor() {
        this.value = 0.0;
    }

    /**
     * Creates a new DoubleBox instance with the specified initial value.
     *
     * @param { double } value - The initial double value to be wrapped
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(value: double) {
        this.value = value;
    }

    /**
     * Creates a new DoubleBox instance with the specified initial value.
     *
     * @param { double } value - The initial double value to be wrapped
     * @returns { DoubleBox } A new DoubleBox instance containing the specified double value
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static box(value: double): DoubleBox {
        return new DoubleBox(value);
    }

    /**
     * Gets the double value wrapped in this DoubleBox.
     *
     * @returns { double } The double value wrapped in this DoubleBox
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get(): double {
        return this.value;
    }

    /**
     * Sets the double value wrapped in this DoubleBox.
     *
     * @param { double } value - The new double value to set
     * @returns { double } The newly set double value
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set(value: double): double {
        return (this.value = value);
    }
}
