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

/**
 * @file Defines Either for ArkTS
 * @kit ArkTS
 */

package std.core;

/**
 * Represents Either class that holds one of Left or Right value
 *
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 */
export abstract class Either<L, R> {
    protected Ldata: L
    protected Rdata: R
    protected isLeft_: boolean

    /**
     * Checks if Either is Left
     *
     * @returns { boolean } If Either is Left
     * @syscap SystemCapability.Utils.Lang
     * @final
     * @FaAndStageModel
     */
    public final isLeft(): boolean {
        return this.isLeft_
    }

    /**
     * Checks if Either is Right
     *
     * @returns { boolean } If Either is Right
     * @syscap SystemCapability.Utils.Lang
     * @final
     * @FaAndStageModel
     */
    public final isRight(): boolean {
        return !this.isLeft()
    }

    /**
     * Checks if Either is Left and returns L data
     *
     * @returns { L } Left data of Either
     * @throws { TypeError } If Either is not Left
     * @syscap SystemCapability.Utils.Lang
     * @final
     * @FaAndStageModel
     */
    public final get left(): L {
        if (this.isLeft()) {
            return this.Ldata
        }
        throw new Error("Cannot read Left value from Right!")
    }

    /**
     * Checks if Either is Right and returns R data
     *
     * @returns { R } Right data of Either
     * @throws { Error } If Either is not Right
     * @syscap SystemCapability.Utils.Lang
     * @final
     * @FaAndStageModel
     */
    public final get right(): R {
        if (this.isRight()) {
            return this.Rdata
        }
        throw new Error("Cannot read Right value from Left!")
    }
}

/**
 * Represents Left class of Either
 *
 * @extends Either<L, R>
 * @syscap SystemCapability.Utils.Lang
 * @final
 * @FaAndStageModel
 */
export final class Left<L, R> extends Either<L, R> {
    private constructor(data: L) {
        this.Ldata = data
        this.isLeft_ = true
    }

    /**
     * Creates a new instance of a Left<L,R> with L data
     *
     * @param { L } data for Left
     * @returns { Left<L,R> } A new Left instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static $_invoke<L, R>(data: L): Left<L, R> {
        return new Left<L,R>(data)
    }
}

/**
 * Represents Right class of Either
 *
 * @extends Either<L, R>
 * @syscap SystemCapability.Utils.Lang
 * @final
 * @FaAndStageModel
 */
export final class Right<L, R> extends Either<L, R> {
    private constructor(data: R) {
        this.Rdata = data
        this.isLeft_ = false
    }

    /**
     * Creates a new instance of a Right<L,R> with R data
     *
     * @param { R } data for Right
     * @returns { Right<L,R> } A new Right instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     */
    public static $_invoke<L, R>(data: R): Right<L, R> {
        return new Right<L,R>(data)
    }
}
