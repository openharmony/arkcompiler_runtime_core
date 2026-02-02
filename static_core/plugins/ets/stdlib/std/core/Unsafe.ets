/*
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

package std.core

/*
  DISCLAIMER

  This API is _unsafe_ and should not be used under normal circumstances.
  The memory management in the native context (and possible clashes with
  the GC in the managed one) is the user's sole reponsibility.
*/

/**
 * @file Defines the Unsafe for ArkTS
 * @kit ArkTS
 */

/**
 * Provides an unsafe native memory operation interface, allowing direct 
 * read and write of data at specified memory addresses.
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export namespace unsafeMemory {
    /**
     * Writes an 8-bit signed integer to the specified memory address.
     * @param { long } address - The memory address to write to.
     * @param { byte } value - The 8-bit signed integer value to write.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function writeInt8(address: long, value: byte) : void;

    /**
     * Write a boolean value to the specified memory address.
     * @param { long } address - The memory address to write to.
     * @param { boolean } value - The boolean value to be written.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function writeBoolean(address: long, value: boolean) : void;

    /**
     * Write a 16-bit signed integer to the specified memory address.
     * @param { long } address - The memory address to write to.
     * @param { short } value - The 16-bit signed integer value to be written.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function writeInt16(address: long, value: short) : void;

    /**
     * Write a 32-bit signed integer to the specified memory address.
     * @param { long } address - The memory address to write to.
     * @param { int } value - The 32-bit signed integer value to be written.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function writeInt32(address: long, value: int) : void;

    /**
     * Write a 32-bit single-precision floating-point value to the specified memory address.
     * @param { long } address - The memory address to write to.
     * @param { float } value - The 32-bit single-precision floating-point value to be written.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function writeFloat32(address: long, value: float) : void;

    /**
     * Write a 64-bit signed integer to the specified memory address.
     * @param { long } address - The memory address to write to.
     * @param { long } value - The 64-bit signed integer value to be written.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function writeInt64(address: long, value: long) : void;

    /**
     * Write a 64-bit double-precision floating-point value to the specified memory address.
     * @param { long } address - The memory address to write to.
     * @param { double } value - The 64-bit double-precision floating-point value to be written.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function writeFloat64(address: long, value: double) : void;

    /**
     * Write a number to the specified memory address.
     * @param { long } address - The memory address to write to.
     * @param { number } value - The numeric value to be written.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function writeNumber(address: long, value: number) : void;

    /**
     * Write a string to the specified memory address.
     * @param { long } address - The memory address to write to.
     * @param { String } value - The string value to be written.
     * @returns { int } The number of bytes written.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function writeString(address: long, value: String) : int;

    /**
     * Read an 8-bit signed integer from the specified memory address.
     * @param { long } address - The memory address to read from.
     * @returns { byte } The 8-bit signed integer value read.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function readInt8(address: long) : byte;

    /**
     * Read a boolean value from the specified memory address.
     * @param { long } address - The memory address to read from.
     * @returns { boolean } The boolean value read.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function readBoolean(address: long) : boolean;

    /**
     * Read a 16-bit signed integer from the specified memory address.
     * @param { long } address - The memory address to read from.
     * @returns { short } The 16-bit signed integer value read.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function readInt16(address: long) : short;

    /**
     * Read a 32-bit signed integer from the specified memory address.
     * @param { long } address - The memory address to read from.
     * @returns { int } The 32-bit signed integer value read.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function readInt32(address: long) : int;

    /**
     * Read a 64-bit signed integer from the specified memory address.
     * @param { long } address - The memory address to read from.
     * @returns { long } The 64-bit signed integer value read.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function readInt64(address: long) : long;

    /**
     * Read a 32-bit single-precision floating-point value from the specified memory address.
     * @param { long } address - The memory address to read from.
     * @returns { float } The 32-bit single-precision floating-point value read.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function readFloat32(address: long) : float;

    /**
     * Read a 64-bit double-precision floating-point value from the specified memory address.
     * @param { long } address - The memory address to read from.
     * @returns { double } The 64-bit double-precision floating-point value read.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function readFloat64(address: long) : double;

    /**
     * Read a number from the specified memory address.
     * @param { long } address - The memory address to read from.
     * @returns { number } The numeric value read.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function readNumber(address: long) : number;

    /**
     * Read a string from the specified memory address.
     * @param { long } address - The memory address to read from.
     * @param { int } size - The number of bytes to read.
     * @returns { String } The string value read.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function readString(address: long, size: int) : String;

    /**
     * Get the number of bytes a specified string occupies in memory.
     * @param { string } value - The string whose size is to be calculated.
     * @returns { int } The number of bytes occupied by the string.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    export native function getStringSizeInBytes(value: string): int;
}
