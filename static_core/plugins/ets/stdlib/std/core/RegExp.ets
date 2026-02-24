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
 * @file Defines the RegExp for ArkTS
 * @kit ArkTS
 */

package std.core

/**
 * The container type used to store named capture groups, with the key being the group name
 *     and the value being a matching string or undefined.
 * @typedef { Record<String, String | undefined> } RegExpGroupsContainer
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
type RegExpGroupsContainer = Record<String, String | undefined>

/**
 * Regular expression result descriptor
 * @extends Array<String | undefined>
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class RegExpResultArray extends Array<String | undefined> {
    /**
     * Read only attribute, indicating whether the match was successful.
     * @type { boolean }
     * @readonly
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    readonly isCorrect: boolean

    private resultRaw_: String = ""
    /** The 0-based index of the match in the string */
    protected index_: int = 0
    /** The original string that was matched against */
    protected input_: String = ""

    private endIndex: int = 0
    private wasProcessed: boolean = false

    // NOTE(shumilov-petr): Make nullable when #14813 will be fixed
    protected indices_: Array<Array<int>> = []

    /** named groups names and indeces */
    protected groupsRaw_: String = ""

    /** named groups names and indeces */
    public groups: RegExpGroupsContainer | undefined = undefined

    constructor(index: int, input: String, result: Array<String | undefined>, indices: Array<Array<int>>) {
        super(0)
        this.isCorrect = true
        this.index_ = index
        this.input_ = input
        this.indices_ = indices

        this.extendTo(result.length, undefined as (String | undefined))
        for (let i: int = 0; i < result.length; i++) {
            this.$_set(i, result.$_get(i))
        }
    }

    protected constructor() {
        super(0)
        this.isCorrect = false
    }

    /**
     * Get the matching result array itself.
     * @returns { Array<String | undefined> } Match result array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get result(): Array<String | undefined> {
        return this
    }

    public postExecProcessing(res: RegExpResultArray, input: String, index: int, hasIndices: boolean): void {
        // NOTE(ekaterinazaytseva): temp solution - will be fixed in #20259
        this.updateGroups()
        this.updateIndicesAndResult(hasIndices)
        this.wasProcessed = true
    }

    /**
     * Returns the first match if "0" is given and the first match exists
     *
     * @param index
     *
     * @returns the first match as string or null
     */
    public $_get(index: String): String {
        if (index == "0") {
            const v = this.$_get(0)
            if (v !== undefined) return v
            throw new Error("result[0] is undefined")
        }
        throw new Error("unsupported field name")
    }

    /**
     * Returns result string by index.
     *
     * @param { int } index Integer index.
     * @returns { String | undefined } resulting string.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public $_get(index: int): String | undefined {
        return super.$_get(index)
    }

    /**
     * Set the matching result at the specified index.
     * @param { int } index The index to be set.
     * @param { String | undefined } val The value to be set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public $_set(index: int, val: String | undefined): void {
        super.$_set(index, val)
    }

    /**
     * Get an array containing the start and end indices of each matching substring.
     * @returns { Array<Array<int>> } Index array.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get indices(): Array<Array<int>> {
        return this.indices_
    }

     /**
     * Returns a string representation of the matching result.
     *
     * @returns { String } a string representing the match result
     * @throws { Error } - When the matching result is empty, throw it.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public toString(): String {
        if (this.length > 0) {
            return super.toString();
        }
        throw new Error("result is empty")
    }

    /**
     * Parses groupsRaw_ to groups
     */
    protected updateGroups(): void {
        throw new Error("not overridden")
    }

    // NOTE(mgroshev): remove it as part of #20259
    public getWasProcessed(): boolean {
        return this.wasProcessed
    }

    private updateIndicesAndResult(hasIndices: boolean): void {
        const data = this.resultRaw_.split(',')
        const n: int = (data.length / 2)

        if (n > this.length) {
            this.extendTo(n, undefined as (String | undefined))
        } else if (n < this.length) {
            this.shrinkTo(n)
        }

        if (hasIndices) {
            this.indices_ = new Array<Array<int>>(n)
            for (let i = 0; i < data.length; i += 2) {
                const index: int = Int.parseInt(data[i], 10)
                const endIndex: int = Int.parseInt(data[i + 1], 10)
                const pos: int = (i / 2);
                if (index == -1 || endIndex == -1) {
                    this.indices_[pos] = [0, 0]
                } else {
                    this.indices_[pos] = [index, endIndex]
                    const str = this.input_.substring(index, endIndex)
                    this.$_set(pos, str)
                }
            }
        } else {
            this.indices_ = []
            for (let i = 0; i < data.length; i += 2) {
                const index = Int.parseInt(data[i], 10)
                const endIndex = Int.parseInt(data[i + 1], 10)
                const pos: int = (i / 2);
                if (index != -1 && endIndex != -1) {
                    const str = this.input_.substring(index, endIndex)
                    this.$_set(pos, str)
                }
            }
        }
    }
}

/**
 * Represents the matching result returned by String.prototype.matchAll(),
 *     or the non iterative result of RegExp. prototype. exec() in global mode.
 * @extends RegExpResultArray
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class RegExpMatchArray extends RegExpResultArray {
    private hasIndex_: boolean = true
    private hasInput_: boolean = true

    /**
     * Creates a RegExpMatchArray instance that contains matching results.
     *
     * @param { int } index The starting index of the matching result.
     * @param { String } input The original string used for matching.
     * @param { Array<String | undefined> } result The string array that matches.
     * @param { Array<Array<int>> } indices The starting and ending index arrays of each matching substring.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(index: int, input: String, result: Array<String | undefined>, indices: Array<Array<int>>) {
        super(index, input, result, indices)
    }

    /**
     * Creates a RegExpMatchArray instance containing matching results.
     *
     * @param { int } index The starting index of the matching result.
     * @param { String } input The original string used for matching.
     * @param { Array<String | undefined> } result The string array that matches.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(index: int, input: String, result: Array<String | undefined>) {
        this(index, input, result, [])
    }

    protected constructor() {
        super()
    }

    /**
     * Gets the starting index of the match result. Returns undefined if invalid.
     * @returns { int | undefined } The starting index of the match.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get index(): int | undefined {
        if (this.hasIndex_) {
            return this.index_
        }
        return undefined
    }

    /**
     * Gets the original string used for matching. Returns undefined if invalid.
     * @returns { String | undefined } The original string.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get input(): String | undefined {
        if (this.hasInput_) {
            return this.input_
        }
        return undefined
    }

    /**
     * Sets the starting index of the match result.
     * @param { int | undefined } val The index value to be set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set index(val: int | undefined) {
        if (val == undefined) {
            this.hasIndex_ = false
            return
        }
        this.hasIndex_ = true
        this.index_ = val
    }

    public set input(val: String | undefined) {
        if (val == undefined) {
            this.hasInput_ = false
            return
        }
        this.hasInput_ = true
        this.input_ = val
    }

    /**
     * Parses groupsRaw_ to groups
     */
    protected override updateGroups(): void {
        if (this.groupsRaw_ == "") {
            this.groups = undefined
            return
        }
        let result = new RegExpGroupsContainer()

        if (!this.hasInput_) {
            return
        }

        for (let groupData of this.groupsRaw_.split(";")) {
            if (groupData.length == 0) {
                continue
            }
            let unitData = groupData.split(",")
            if (unitData.length != 3) {
                throw new Error("internal error: invalid group data")
            }
            let index = Int.parseInt(unitData[1], 10)
            let endIndex = Int.parseInt(unitData[2], 10)
            if (index >= 0 && endIndex >= 0) {
                if (endIndex > this.input_.length || index > endIndex) {
                    throw new Error("internal error: incorrect substring indexes")
                }
                result.set(unitData[0], this.input!.substring(index, endIndex))
            } else {
                result.set(unitData[0], undefined)
            }
        }
        if (result.size == 0) {
            this.groups = undefined
            return
        }
        this.groups = result
    }
}

/**
 * Represents the return result of exec(), containing detailed information of a single match.
 * @extends RegExpResultArray
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class RegExpExecArray extends RegExpResultArray {
    /**
     * Creates a RegExpExecArray instance containing match results.
     *
     * @param { int } index The starting index of the match result.
     * @param { String } input The original string used for matching.
     * @param { Array<String | undefined> } result The array of matched strings.
     * @param { Array<Array<int>> } indices The start and end index arrays of each matching substring.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(index: int, input: String, result: Array<String | undefined>, indices: Array<Array<int>>) {
        super(index, input, result, indices)
    }

    /**
     * Creates a RegExpExecArray instance containing match results (without indices).
     *
     * @param { int } index The starting index of the match result.
     * @param { String } input The original string used for matching.
     * @param { Array<String | undefined> } result The array of matched strings.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    constructor(index: int, input: String, result: Array<String | undefined>) {
        this(index, input, result, [])
    }

    protected constructor() {
        super()
    }

    /**
     * Gets the starting index of the match result.
     * @returns { int } The starting index of the match.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get index(): int {
        return this.index_
    }

    /**
     * Gets the original string used for matching.
     * @returns { String } The original string.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public get input(): String {
        return this.input_
    }

    /**
     * Sets the starting index of the match result.
     * @param { int } val The index value to be set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set index(val: int) {
        this.index_ = val
    }

    /**
     * Sets the original string used for matching.
     * @param { String } val The string value to be set.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public set input(val: String) {
        this.input_ = val
    }

    /**
     * Parses groupsRaw_ to groups
     */
    protected override updateGroups(): void {
        if (this.groupsRaw_ == "") {
            this.groups = undefined
            return
        }
        let result = new RegExpGroupsContainer()

        for (let groupData of this.groupsRaw_.split(";")) {
            if (groupData.length == 0) {
                continue
            }
            let unitData = groupData.split(",")
            if (unitData.length != 3) {
                throw new Error("internal error: invalid group data")
            }
            let index = Int.parseInt(unitData[1], 10)
            let endIndex = Int.parseInt(unitData[2], 10)
            if (index >= 0 && endIndex >= 0) {
                if (endIndex > this.input_.length || index > endIndex) {
                    throw new Error("internal error: incorrect substring indexes index = " + index + " endIndex = " + endIndex + 
                    " length = " + this.input_.length)
                }
                result.set(unitData[0], this.input.substring(index, endIndex))
            } else {
                result.set(unitData[0], undefined)
            }
        }
        if (result.size == 0) {
            this.groups = undefined
            return
        }
        this.groups = result
    }
}

class GlobalRegExpStringIterator implements IterableIterator<RegExpMatchArray> {
    private regexp: RegExp
    private string: String
    private returned: boolean

    constructor(regexp: RegExp, string: String) {
        this.regexp = regexp
        this.string = string
        this.returned = false
    }

    override next(): IteratorResult<RegExpMatchArray> {
        if (this.returned) {
            return new IteratorResult<RegExpMatchArray>();
        }
        let match = this.regexp.matchInternal(this.string)
        if (match == null) {
            this.returned = true;
            return new IteratorResult<RegExpMatchArray>()
        }
        let matchStr = match.$_get(0)
        if (matchStr == "") {
            this.regexp.lastIndex = RegExp.advanceStringIndex(this.string, this.regexp.lastIndex, this.regexp.unicode)
        }
        return new IteratorResult<RegExpMatchArray>(match)
    }

    override $_iterator(): IterableIterator<RegExpMatchArray> {
        return this
    }
}

class RegExpStringIterator implements IterableIterator<RegExpMatchArray> {
    private regexp: RegExp
    private string: String
    private returned: boolean

    constructor(regexp: RegExp, string: String) {
        this.regexp = regexp
        this.string = string
        this.returned = false
    }

    override next(): IteratorResult<RegExpMatchArray> {
        let match = this.regexp.matchInternal(this.string)
        if (match == null || this.returned) {
            return new IteratorResult<RegExpMatchArray>()
        }
        this.returned = true
        return new IteratorResult<RegExpMatchArray>(match)
    }

    override $_iterator(): IterableIterator<RegExpMatchArray> {
        return this
    }
}

/**
 * Regular expression
 * @extends Object
 * @syscap SystemCapability.Utils.Lang
 * @FaAndStageModel
 * @since 24
 */
export class RegExp extends Object {
    private pattern_: String = ""
    public lastIndex: int = 0
    private source_: String = ""
    private flags_: String = ""
    private flagsSorted_: boolean = false

    /**
     * Gets the flags of the regular expression.
     * @return { String } A string containing the flags used for the regular expression.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get flags(): String {
        if (!this.flagsSorted_) {
            this.flags_ = this.flags_.split("").sort().join("")
            this.flagsSorted_ = true
        }
        return this.flags_
    }

    /**
     * Gets the global flag, indicating whether to perform global matching.
     * @return { boolean } `true` if global matching is enabled, `false` otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get global(): boolean {
        return this.flags_.contains("g", 0)
    }

    /**
     * Gets the dotAll flag, indicating whether '.' matches newline characters.
     * @return { boolean } `true` if dotAll is enabled, `false` otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get dotAll(): boolean {
        return this.flags_.contains("s", 0)
    }

    /**
     * Gets the hasIndices flag, indicating whether to include indices of matching substrings.
     * @return { boolean } `true` if indices are included in the match result, `false` otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get hasIndices(): boolean {
        return this.flags_.contains("d", 0)
    }

    /**
     * Gets the ignoreCase flag, indicating whether to ignore case.
     * @return { boolean } `true` if case is ignored, `false` otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get ignoreCase(): boolean {
        return this.flags_.contains("i", 0)
    }

    /**
     * Gets the multiline flag, indicating whether to perform multiline matching.
     * @return { boolean } `true` if multiline matching is enabled, `false` otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get multiline(): boolean {
        return this.flags_.contains("m", 0)
    }

    /**
     * Gets the sticky flag, indicating whether to perform sticky matching.
     * @return { boolean } `true` if sticky matching is enabled, `false` otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get sticky(): boolean {
        return this.flags_.contains("y", 0)
    }

    /**
     * Gets the unicode flag, indicating whether to enable Unicode mode.
     * @return { boolean } `true` if Unicode mode is enabled, `false` otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get unicode(): boolean {
        return this.flags_.contains("u", 0)
    }

    /**
     * Gets the unicodeSets flag, indicating whether Unicode set mode is enabled.
     * @return { boolean } `true` if Unicode set mode is enabled, `false` otherwise.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get unicodeSets(): boolean {
        return this.flags_.contains("v", 0)
    }

    /**
     * Returns a string containing the source text of this regular expression
     * @return { String } The source of the regular expression.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    get source(): String {
        return this.source_
    }

    /**
     * Constructs a new RegExp using pattern and flags
     *
     * @param { String } pattern description of a pattern
     * @param { String } [flags] description of flags to be used
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(pattern: String, flags?: String) {
        RegExp.init(this, pattern, flags === undefined ? "" : flags!)
    }

    /**
     * Constructs a new RegExp by other RegExp. It uses other RegExp's flags if flags aren't provided
     *
     * @param { RegExp } regexp other regexp
     * @param { String } [flags] description of flags to be used
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(regexp: RegExp, flags?: String) {
        RegExp.init(this, regexp.pattern_, flags === undefined ? regexp.flags : flags!)
    }

    /**
     * Constructs a new RegExp by RegExp or string
     *
     * @param { RegExp | String } regexp The pattern of the regular expression or another RegExp instance.
     * @param { String } [flags] description of flags to be used.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public constructor(regexp: RegExp | String, flags?: String) {
        if (regexp instanceof String) {
            RegExp.init(this, regexp as String, flags === undefined ? "" : flags!)
        } else {
            let re = regexp as RegExp
            RegExp.init(this, re.pattern_, flags === undefined ? re.flags : flags!)
        }
    }

    /**
     * Compiles a regular expression
     * @returns { RegExp } Returns the compiled RegExp object.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public native compile(): RegExp;

    /**
     * Parse a regular expression
     */
    private native static parse(pattern: String, flags: String): void

    /**
     * Recompiles a regular expression with new source and flags after the RegExp object has already been created
     *
     * @param { String } pattern the text of the regular expression.
     * @param { String } [flags] any combination of flag values.
     * @returns { RegExp } Returns the recompiled RegExp object itself.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public compile(pattern: String, flags?: String): RegExp {
        this.pattern_ = pattern
        this.flags_ = RegExp.normalizeFlags(pattern, flags === undefined ? "" : flags!)
        this.flagsSorted_ = false
        this.source_ = RegExp.escapePattern(pattern)
        return this.compile()
    }

    private native execImpl(pattern: String, flags: String, str: String, patternLength: int, strLength: int, lastIndex: int, hasSlashU: boolean): RegExpExecArray;

    private native matchImpl(pattern: String, flags: String, str: String, patternLength: int, strLength: int, lastIndex: int, hasSlashU: boolean): RegExpMatchArray;

    /**
     * Executes a match search on the string starting from the specified index.
     *
     * @param { String } str The string to match.
     * @param { int } index The index to start matching from.
     * @returns { RegExpExecArray | null } If match succeeds, returns an array containing match results,
     *     otherwise returns null.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public exec(str: String, index: int): RegExpExecArray | null {
        const hasSlashU = str.contains("\\u", 0) || this.pattern_.contains("\\u", 0)
        const res = this.execImpl(this.pattern_, this.flags_, str, this.pattern_.length, str.length, index, hasSlashU)
        if (res.isCorrect) {
            res.input = str
            if (!res.getWasProcessed() && res.isCorrect) {
                res.postExecProcessing(res, str, res.index, this.hasIndices)
            }
            return res
        } else {
            return null
        }
    }

    /**
     * Executes a search for a match in a specified string and returns a result array
     *
     * @param { String } str The string to match.
     * @returns { RegExpExecArray | null } If match succeeds, returns an array containing match results,
     *     otherwise returns null.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public exec(str: String): RegExpExecArray | null {
        return this.exec(str, this.lastIndex)
    }

    private matchInternal(str: String, index: int): RegExpMatchArray | null {
        const hasSlashU = str.contains("\\u", 0) || this.pattern_.contains("\\u", 0)
        const res = this.matchImpl(this.pattern_, this.flags_, str, this.pattern_.length, str.length, index, hasSlashU)
        if (res.isCorrect) {
            res.input = str
            if (!res.getWasProcessed() && res.isCorrect) {
                res.postExecProcessing(res, str, res.index!, this.hasIndices)
            }
            return res
        } else {
            return null
        }
    }

    /**
     * Internal method, executes match starting from lastIndex.
     *
     * @param { String } str The string to match.
     * @returns { RegExpMatchArray | null } If match succeeds, returns a match result array, otherwise returns null.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public matchInternal(str: String): RegExpMatchArray | null {
        return this.matchInternal(str, this.lastIndex)
    }

    /**
     * Searches for matches in the string and returns the starting index of the match.
     *
     * @param { String } str The string to search.
     * @returns { int } If a match is found, returns its starting index; otherwise returns -1.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public search(str: String): int {
        let previousLastIndex: int = this.lastIndex
        this.lastIndex = 0
        let result = this.exec(str)
        this.lastIndex = previousLastIndex
        if (result == null) {
            return -1
        }
        return result.index
    }

    /**
     * Replaces matched substrings in the string using a replacement string.
     *
     * @param { String } str The original string.
     * @param { String } replaceValue The string used for replacement.
     * @returns { String } The new string after replacement.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public replace(str: String, replaceValue: String): String {
        if (this.global) {
            this.lastIndex = 0
        }
        let results = new Array<RegExpExecArray>()
        let done = false
        while (!done) {
            let result = this.exec(str)
            if (result == null) {
                break;
            }
            results.push(result)
            if (!this.global) {
                break;
            }
            let matchStr = result.$_get(0)
            if (matchStr == "") {
                this.lastIndex = RegExp.advanceStringIndex(str, this.lastIndex, this.unicode)
            }
        }
        let lengthS = str.getLength()
        let accumulatedResult = new StringBuilder()
        let nextSourcePosition = 0
        for (let i = 0; i < results.length; ++i) {
            let result = results.at(i)!;
            let nCaptures = result.length;
            let matched = result.$_get(0)!;
            let matchLength = matched.getLength();
            let position = max(min(result.index, lengthS), 0);
            let captures = new containers.UndefinableStringArray(nCaptures);
            for (let n = 1; n < nCaptures; n++) {
                let capN = result.$_get(n);
                captures.pushBack(capN)
            }
            let replacement = String.getSubstitution(matched, str, position as int, captures.toArray(), result.groups, replaceValue)
            if (position >= nextSourcePosition) {
                accumulatedResult.append(str.substring(nextSourcePosition, position)).append(replacement)
                nextSourcePosition = position + matchLength
            }
        }
        if (nextSourcePosition >= lengthS) {
            return accumulatedResult.toString()
        }
        return accumulatedResult.append(str.substring(nextSourcePosition)).toString()
    }

    /**
     * Replaces matched substrings in the string using a function.
     *
     * @param { String } str The original string.
     * @param { function } replacer A function used to generate new substrings.
     * @returns { String } The new string after replacement.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public replace(str: String, replacer: (substr: String, args: Object[]) => String): String {
        if (this.global) {
            this.lastIndex = 0
        }
        let results = new Array<RegExpExecArray>()
        let done = false
        while (!done) {
            let result = this.exec(str)
            if (result == null) {
                break;
            }
            results.push(result)
            if (!this.global) {
                break;
            }
            let matchStr = result.$_get(0)
            if (matchStr == "") {
                this.lastIndex = RegExp.advanceStringIndex(str, this.lastIndex, this.unicode)
            }
        }
        let lengthS = str.getLength()
        let accumulatedResult = new StringBuilder()
        let nextSourcePosition = 0
        for (let i = 0; i < results.length; ++i) {
            let result = results.at(i)!;
            let nCaptures = result.length;
            let matched = result.$_get(0)!;
            let matchLength = matched.getLength();
            let position = (max(min(result.index, lengthS), 0));
            let args = new containers.UndefinableObjectArray(nCaptures);
            for (let n = 1; n < nCaptures; n++) {
                let capN = result.$_get(n);
                args.pushBack(capN)
            }
            let namedCaptures: Object | undefined = result.groups;
            args.pushBack(position.toDouble());
            args.pushBack(str)
            if (namedCaptures != undefined) {
                args.pushBack(namedCaptures)
            }
            let replacement = replacer(matched, args.toArray());
            if (position >= nextSourcePosition) {
                accumulatedResult.append(str.substring(nextSourcePosition, position)).append(replacement)
                nextSourcePosition = position + matchLength
            }
        }
        if (nextSourcePosition >= lengthS) {
            return accumulatedResult.toString()
        }
        return accumulatedResult.append(str.substring(nextSourcePosition)).toString()
    }

    /**
     * Splits the string into an array of substrings using the regular expression.
     *
     * @param { String } str The string to split.
     * @param { Int | undefined } limit Limits the maximum length of the returned array.
     * @returns { String[] } The array of substrings after splitting.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public split(str: String, limit: Int | undefined): String[] {
        let unicodeMatching = this.unicode
        let newFlags = this.sticky ? this.flags : this.flags + "y";
        let splitter = new RegExp(this.pattern_, newFlags)
        let lim: long;
        if (limit == undefined) {
            lim = (1 << 32) -1
        } else if (limit == 0) {
            return new String[0]
        } else {
            lim = limit!.toLong()
        }
        let size = str.getLength()
        if (size == 0) {
            let z = splitter.exec(str)
            if (z != null) {
                return new String[0]
            }
            return [str]
        }
        let splittedStrings = new containers.UndefinableStringArray()
        let lastStart = 0
        for (let lastEnd: int = 0; lastEnd < size;) {
            splitter.lastIndex = lastEnd
            let z = splitter.exec(str)
            if (z == null) {
                lastEnd = RegExp.advanceStringIndex(str, lastEnd, unicodeMatching)
            } else {
                let separatorRight = (splitter.lastIndex < size) ? splitter.lastIndex : size
                if (separatorRight == lastStart) {
                    lastEnd = RegExp.advanceStringIndex(str, lastEnd, unicodeMatching)
                } else {
                    let substr = str.substring(lastStart, lastEnd)
                    splittedStrings.pushBack(substr)
                    if (splittedStrings.size() == lim) {
                        return splittedStrings.toArray() as String[]
                    }
                    lastStart = separatorRight
                    let numberOfCaptures = z.length;
                    for (let i = 1; i < numberOfCaptures; ++i) {
                        let nextCapture = z.$_get(i)
                        splittedStrings.pushBack(nextCapture)
                        if (splittedStrings.size() == lim) {
                            return splittedStrings.toArray() as String[]
                        }
                    }
                    lastEnd = lastStart
                }
            }
        }
        let substr = str.substring(lastStart, size)
        splittedStrings.pushBack(substr)
        let res = splittedStrings.toArray() as String[]
        return res
    }

    /**
     * Searches the string for matches and returns all matches as an array.
     *
     * @param { String } str The string to match.
     * @returns { RegExpMatchArray | null } If matches are found, returns an array containing all matches,
     *     otherwise returns null.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public match(str: String): RegExpMatchArray | null {
        if (!this.global) {
            return this.matchInternal(str)
        }
        this.lastIndex = 0;
        let matches = new containers.UndefinableStringArray();
        let indices = new Array<Array<int>>();
        let isUndefined = new Array<boolean>();
        let n = 0
        while (true) {
            let result = this.matchInternal(str)
            if (result == null) {
                if (n == 0) {
                    return null
                }
                let matchArray = new RegExpMatchArray(-1, "", matches.toUndefinableArray(), indices)
                matchArray.index = undefined
                matchArray.input = undefined
                return matchArray
            }
            else {
                let matchStr = result.$_get(0)
                matches.pushBack(matchStr)
                indices = indices.concat(result.indices)
                if (matchStr == "") {
                    this.lastIndex = RegExp.advanceStringIndex(str, this.lastIndex, this.unicode)
                }
                n++
            }
        }
    }

    /**
     * Returns an iterator that iterates over all matches in the string.
     *
     * @param { String } str The string to match.
     * @returns {IterableIterator<RegExpMatchArray>} An iterator that iterates over all match results.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public matchAll(str: String): IterableIterator<RegExpMatchArray> {
        let matcher = new RegExp(this)
        matcher.lastIndex = this.lastIndex
        if (matcher.global) {
            return new GlobalRegExpStringIterator(matcher, str)
        } else {
            return new RegExpStringIterator(matcher, str)
        }
    }

    /**
     * Executes a search for a match between a regular expression and specified string
     *
     * @param { String } str the string against which to match the regular expression.
     * @returns { boolean } If the string has matches, returns true; otherwise returns false.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public test(str: String): boolean {
        let res = this.exec(str)
        if (!res) {
            return false
        }
        return res.isCorrect
    }

    /**
     * Returns a string representing the regular expression.
     *
     * @returns { String } The string representation of the regular expression.
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public override toString(): String {
        return "/" + this.source + "/" + this.flags
    }

    /**
     * Advances the string index according to the Unicode flag.
     *
     * @param { String } s Original string.
     * @param { int } index Current index.
     * @param { boolean } unicode Whether to enable Unicode mode.
     * @returns { int } Next index.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static advanceStringIndex(s: String, index: int, unicode: boolean): int {
        const HIGH_SURROGATE_START = 0xD800;
        const HIGH_SURROGATE_END   = 0xDBFF;
        const LOW_SURROGATE_START  = 0xDC00;
        const LOW_SURROGATE_END    = 0xDFFF;
        const length = s.getLength();
        if (!unicode) {
            return index + 1;
        }
        if (index + 1 >= length) {
            return index + 1;
        }

        const hi = s.charCodeAt(index);
        if (hi < HIGH_SURROGATE_START || hi > HIGH_SURROGATE_END) {
            return index + 1;
        }
        
        const lo = s.charCodeAt(index + 1);
        if (lo >= LOW_SURROGATE_START && lo <= LOW_SURROGATE_END) {
            return index + 2;
        }
        return index + 1;
    }

    /**
     * RegExp constructor call signature, used to create new RegExp instances.
     * @param { String } pattern Regular expression pattern.
     * @param { String } [flags] Regular expression flags.
     * @returns { RegExp } Newly created RegExp instance.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static $_invoke(pattern: String, flags?: String) : RegExp {
        return new RegExp(pattern, flags)
    }

    /**
     * RegExp constructor call signature, used to create new RegExp instances.
     * @param { RegExp | String } pattern Regular expression pattern or another RegExp instance.
     * @param { String } [flags] Regular expression flags.
     * @returns { RegExp } Newly created RegExp instance.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static $_invoke(pattern: RegExp | String, flags?: String) : RegExp {
        return new RegExp(pattern, flags)
    }

    /**
     * RegExp constructor call signature, used to create new RegExp instances.
     * @param { RegExp | String } pattern Regular expression pattern or another RegExp instance.
     * @returns { RegExp } Newly created RegExp instance.
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @FaAndStageModel
     * @since 24
     */
    public static $_invoke(pattern: RegExp | String) : RegExp {
        return new RegExp(pattern)
    }

    private static escapePattern(pattern: String): String {
        if (pattern == "") {
            return "(?:)"
        }
        let s = pattern.replaceAll("/", "\\/")
        return s.replaceAll("\\", "\\")
    }

    private static init(re: RegExp, pattern: String, flags: String): void {
        re.pattern_ = pattern
        re.flags_ = RegExp.normalizeFlags(pattern, flags)
        re.source_ = RegExp.escapePattern(pattern)
        RegExp.parse(pattern, re.flags_)
    }

    private static normalizeFlags(pattern: String, flags: String): String {
        let result = flags
        if (!pattern.isWellFormed()) {
            result = result.replaceAll('u', '');
        }
        return result
    }
}
