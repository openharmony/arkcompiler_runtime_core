/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

package escompat

type RegExpGroupsContainer = Record<String, String | undefined>

/**
 * Regular expression result descriptor
 */
class RegExpResultArray /*extends Array<String>*/ {
    /** true if match present */
    readonly isCorrect: boolean
    /** result itself */
    protected result_: Array<String> = []
    private resultRaw_: String = ""
    /** The 0-based index of the match in the string */
    protected index_: number = 0
    /** The original string that was matched against */
    protected input_: String = ""

    internal endIndex: number = 0

    // NOTE(shumilov-petr): Make nullable when #14813 will be fixed
    protected indices_: Array<Array<number>> = []

    /** named groups names and indeces */
    protected groupsRaw_: String = ""

    /** named groups names and indeces */
    public groups: RegExpGroupsContainer | undefined = undefined

    protected isUndefined: Array<boolean> = []

    constructor(index: int, input: String, result: Array<String>, indices: Array<Array<number>>) {
        this.isCorrect = true
        this.index_ = index
        this.input_ = input
        this.result_ = result
        this.indices_ = indices
    }

    constructor(index : int, input : String, result : Array<String>, indices : Array<Array<number>>,
                isUndefined : Array<boolean>) {
        this.isCorrect = true
        this.index_ = index
        this.input_ = input
        this.result_ = result
        this.indices_ = indices
        this.isUndefined = isUndefined
    }

    internal constructor() {

    }

    private setNumberedField(index: number, value: String): void {
        if (index > 9 || index <= 0) {
            return
        }
        if (index == 1) RegExp.$1_ = value
        else if (index == 2) RegExp.$2_ = value
        else if (index == 3) RegExp.$3_ = value
        else if (index == 4) RegExp.$4_ = value
        else if (index == 5) RegExp.$5_ = value
        else if (index == 6) RegExp.$6_ = value
        else if (index == 7) RegExp.$7_ = value
        else if (index == 8) RegExp.$8_ = value
        else if (index == 9) RegExp.$9_ = value
    }

    internal postExecProcessing(res: RegExpResultArray, input: String, index: number, hasIndices: boolean): void {
        if (!res.isCorrect) {
            return
        }
        RegExp.$1_ = ""
        RegExp.$2_ = ""
        RegExp.$3_ = ""
        RegExp.$4_ = ""
        RegExp.$5_ = ""
        RegExp.$6_ = ""
        RegExp.$7_ = ""
        RegExp.$8_ = ""
        RegExp.$9_ = ""
        this.updateGroups()
        this.updateIndicesAndResult(hasIndices)
        RegExp.input_ = input
        RegExp.leftContext_ = input.substring(0, index)
        RegExp.rightContext_ = input.substring(index + res.result[0].length)
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
            if (this.result_.length > 0) {
                return this.result_[0]
            }
            throw new Error("result is empty")
        }
        throw new Error("unsupported field name")
    }

    /**
     * Returns result string by index
     *
     * @param index
     *
     * @returns resulting string
     */
    public $_get(index: int): String | undefined {
        if (index < this.isUndefined.length && this.isUndefined[index]) {
            return undefined
        }
        return this.result_[index];
    }

    /**
     * Returns result string by index
     *
     * @param index
     *
     * @returns resulting string
     */
    public $_get(index: number): String | undefined {
        return this.$_get(index.toInt());
    }

    /**
     * Sets value by index
     *
     * @param index
     *
     * @param val
     */
    public $_set(index: int, val: String) {
        this.result_[index] = val
    }

    /**
     * Sets value by index
     *
     * @param index
     *
     * @param val
     */
    public $_set(index: number, val: String) {
        return this.$_set(index.toInt(), val);
    }

    get length(): int {
        return Double.toInt(this.result_.length)
    }

    public get result(): Array<String> {
        return this.result_
    }

    public get indices(): Array<Array<number>> {
        return this.indices_
    }

    /**
     * Returns a string representing the match result
     *
     * @returns a string representing the match result
     */
    public toString(): String {
        if (this.result_.length > 0) {
            return this.result_.toString();
        }
        throw new Error("result is empty")
    }

    /**
     * Parses groupsRaw_ to groups
     */
    internal updateGroups(): void {
        throw new Error("not overridden")
    }

    private processStaticFields(index: number, str: String): void {
        if (index == 0) {
            RegExp.lastMatch_ = str
        }
        if (index == this.result_.length - 1) {
            RegExp.lastParen_ = str
        }
        this.setNumberedField(index, str)
    }

    internal updateIndicesAndResult(hasIndices: boolean): void {
        let data = this.resultRaw_.split(',')
        this.result_ = new Array<String>(data.length / 2)
        this.isUndefined = new Array<boolean>(data.length / 2)
        if (hasIndices) {
            this.indices_ = new Array<Array<number>>(data.length / 2)
            for (let i = 0; i < data.length; i+= 2) {
                const index = Number.parseInt(data[i])
                const endIndex = Number.parseInt(data[i + 1])
                if (index == -1 || endIndex == -1) {
                    this.indices_[i / 2] = [0, 0]
                    this.isUndefined[i / 2] = true
                } else {
                    this.indices_[i / 2] = [index, endIndex]
                    this.result_[i / 2] = this.input_.substring(index, endIndex)
                    this.processStaticFields(i / 2, this.result_[i / 2])
                    this.isUndefined[i / 2] = false
                }
            }
        } else {
            this.indices_ = []
            for (let i = 0; i < data.length; i+= 2) {
                const index = Number.parseInt(data[i])
                const endIndex = Number.parseInt(data[i + 1])
                if (index != -1 && endIndex != -1) {
                    this.result_[i / 2] = this.input_.substring(index, endIndex)
                    this.processStaticFields(i / 2, this.result_[i / 2])
                    this.isUndefined[i / 2] = false
                } else {
                    this.isUndefined[i / 2] = true
                }
            }
        }
    }

    /**
     * Appends one element to result
     *
     * @param string to add
     */
    public push(newElem: string): void {
        this.result_.push(newElem)
    }

    /**
     * Removes and returns the last match result in the array.
     * Simulates the behavior of Array.prototype.pop().
     *
     * @returns The last matched string if present; otherwise, undefined.
     */
    public pop(): String | undefined {
        return this.result_.pop()
    }
}

export class RegExpMatchArray extends RegExpResultArray {
    private hasIndex_: boolean = true
    private hasInput_: boolean = true

    constructor(index : int, input : String, result : Array<String>, indices : Array<Array<number>>,
                isUndefined : Array<boolean>) {
        super(Double.toInt(index), input, result, indices, isUndefined)
    };

    constructor(index: int, input: String, result: Array<String>, indices: Array<Array<number>>) {
        super(Double.toInt(index), input, result, indices)
    }

    constructor(index: int, input: String, result: Array<String>) {
        this(Double.toInt(index), input, result, [])
    }

    constructor(index: number, input: String, result: Array<String>, indices: Array<Array<number>>) {
        this(Double.toInt(index), input, result, indices)
    }
    
    constructor(index: number, input: String, result: Array<String>) {
        this(Double.toInt(index), input, result)
    }

    internal constructor() {
        super()
    }

    public get index(): number | undefined {
        if (this.hasIndex_) {
            return this.index_
        }
        return undefined
    }

    public get input(): String | undefined {
        if (this.hasInput_) {
            return this.input_
        }
        return undefined
    }

    public set index(val: number | undefined) {
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
    internal override updateGroups(): void {
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
            let index = Double.parseInt(unitData[1])
            let endIndex = Double.parseInt(unitData[2])
            if (index >= 0 && endIndex >= 0) {
                if (endIndex > this.input_.length || index >= endIndex) {
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

export class RegExpExecArray extends RegExpResultArray {
    constructor(index: int, input: String, result: Array<String>, indices: Array<Array<number>>) {
        super(Double.toInt(index), input, result, indices)
    }

    constructor(index: int, input: String, result: Array<String>) {
        this(Double.toInt(index), input, result, [])
    }

    constructor(index: number, input: String, result: Array<String>, indices: Array<Array<number>>) {
        this(Double.toInt(index), input, result, indices)
    }
    
    constructor(index: number, input: String, result: Array<String>) {
        this(Double.toInt(index), input, result)
    }

    internal constructor() {
        super()
    }

    public get index(): number {
        return this.index_
    }

    public get input(): String {
        return this.input_
    }

    public set index(val: number) {
        this.index_ = val
    }

    public set input(val: String) {
        this.input_ = val
    }

    /**
     * Parses groupsRaw_ to groups
     */
    internal override updateGroups(): void {
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
            let index = Double.parseInt(unitData[1])
            let endIndex = Double.parseInt(unitData[2])
            if (index >= 0 && endIndex >= 0) {
                if (endIndex > this.input_.length || index >= endIndex) {
                    throw new Error("internal error: incorrect substring indexes")
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

    constructor(regexp: RegExp, string: String) {
        this.regexp = regexp
        this.string = string
    }

    override next(): IteratorResult<RegExpMatchArray> {
        let match = this.regexp.matchInternal(this.string)
        if (match == null) {
            return new IteratorResult<RegExpMatchArray>()
        }
        let matchStr = match.$_get(0)
        if (matchStr == "") {
            let nextIndex = RegExp.advanceStringIndex(this.string, this.regexp.lastIndex, this.regexp.unicode)
            this.regexp.lastIndex = nextIndex
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
 */
export class RegExp extends Object {
    private pattern_: String = ""
    public lastIndex: number = 0
    private source_: String = ""
    private flags_: String = ""

    private static input_: String = ""
    private static lastMatch_: String = ""
    private static lastParen_: String = ""
    private static leftContext_: String = ""
    private static rightContext_: String = ""
    private static $1_: String = ""
    private static $2_: String = ""
    private static $3_: String = ""
    private static $4_: String = ""
    private static $5_: String = ""
    private static $6_: String = ""
    private static $7_: String = ""
    private static $8_: String = ""
    private static $9_: String = ""

    // NOTE(shumilov-petr): Removing the following setter leads to unspecified behaviour of es2panda
    // The negative test '02.lexical_elements/07.keywords/types_n_12.ets' gets failed if remove this setter
    set __removeMe(a: int) {}

    /** Flags */
    get flags(): String {
        return this.flags_
    }

    /**
     * Has the value true if the regular expression should be tested against
     * all possible matches in a string
     */
    get global(): boolean {
        return this.flags_.contains("g", 0)
    }

    /**
     * Has the value true if the dot special character (.) should additionally match
     * the line terminator characters in a string
     */
    get dotAll(): boolean {
        return this.flags_.contains("s", 0)
    }

    /**
     * Has the value true if the result of a regular expression match should contain
     * the start and end indices of the substrings of each capture group
     */
    get hasIndices(): boolean {
        return this.flags_.contains("d", 0)
    }

    /**
     * Has the value true if case should be ignored while attempting a match in a string
     */
    get ignoreCase(): boolean {
        return this.flags_.contains("i", 0)
    }

    /**
     * Has the value true if a multiline input string should be treated as multiple lines
     */
    get multiline(): boolean {
        return this.flags_.contains("m", 0)
    }

    /**
     * Has the value true if the regex attempts to match the target string only
     * from the index indicated by the lastIndex property
     */
    get sticky(): boolean {
        return this.flags_.contains("y", 0)
    }

    /**
     * Has the value true if 'u' flag is used
     */
    get unicode(): boolean {
        return this.flags_.contains("u", 0)
    }

    /**
     * Has the value true if 'v' flag is used
     */
    get unicodeSets(): boolean {
        return this.flags_.contains("v", 0)
    }

    /**
     * Returns a string containing the source text of this regular expression
     */
    get source(): String {
        return this.source_
    }

    /**
     * Returns the string against which a regular expression is matched
     */
    static get input(): String {
        return RegExp.input_
    }

    /**
     * Returns the string against which a regular expression is matched
     */
    static get $_(): String {
        return RegExp.input_
    }

    /**
     * Returns the last matched substring
     */
    static get lastMatch(): String {
        return RegExp.lastMatch_
    }

    /**
     * Returns the last parenthesized substring match, if any
     */
    static get lastParen(): String {
        return RegExp.lastParen_
    }

    /**
     * Returns the substring preceding the most recent match
     */
    static get leftContext(): String {
        return RegExp.leftContext_
    }

    /**
     * Returns the substring following the most recent match
     */
    static get rightContext(): String {
        return RegExp.rightContext_
    }

    /**
     * Returns different aliased properties
     */
    static $_get(key: String): String {
        if (key == "$_") {
            return RegExp.input_
        }
        if (key == "$'") {
            return RegExp.rightContext_
        }
        if (key == "$`") {
            return RegExp.leftContext_
        }
        if (key == "$+") {
            return RegExp.lastParen_
        }
        if (key == "$&") {
            return RegExp.lastMatch_
        }
        throw new Error("unsupported field name")
    }

    /**
     * Static accessor properties return parenthesized substring matches
     */
    static get $1(): String {
        return RegExp.$1_
    }

    static get $2(): String {
        return RegExp.$2_
    }

    static get $3(): String {
        return RegExp.$3_
    }

    static get $4(): String {
        return RegExp.$4_
    }

    static get $5(): String {
        return RegExp.$5_
    }

    static get $6(): String {
        return RegExp.$6_
    }

    static get $7(): String {
        return RegExp.$7_
    }

    static get $8(): String {
        return RegExp.$8_
    }

    static get $9(): String {
        return RegExp.$9_
    }

    /**
     * Constructs a new RegExp using pattern and flags
     *
     * @param pattern description of a pattern
     *
     * @param flags description of flags to be used
     */
    public constructor(pattern: String, flags?: String) {
        this.init(pattern, flags)
    }

    /**
     * Constructs a new RegExp by other RegExp. It uses other RegExp's flags if flags aren't provided
     *
     * @param regexp other regexp
     *
     * @param flags description of flags to be used
     */
    public constructor(regexp: RegExp, flags?: String) {
        this.init(regexp.pattern_, flags === undefined ? regexp.flags : flags!)
    }

    /**
     * Constructs a new RegExp by RegExp or string
     *
     * @param regexp other regexp
     *
     * @param flags description of flags to be used
     */
    public constructor(regexp: RegExp | String, flags?: String) {
        if (regexp instanceof String) {
            this.init(regexp as String, flags)
        } else {
            this.init(regexp as RegExp, flags)
        }
    }

    /**
     * Compiles a regular expression
     */
    public native compile(): RegExp;

    /**
     * Parse a regular expression
     */
    public native static parse(pattern: String): String | undefined;

    /**
     * Recompiles a regular expression with new source and flags after the RegExp object has already been created
     *
     * @param pattern the text of the regular expression
     *
     * @param flags any combination of flag values
     */
    public compile(pattern: String, flags?: String): RegExp {
        this.pattern_ = pattern
        this.flags_ = flags === undefined ? "" : flags!
        this.source_ = RegExp.escapePattern(pattern)
        return this.compile()
    }

    private native execImpl(pattern: String, flags: String, str: String, patternLength: number, strLength: number, lastIndex: number, hasSlashU: boolean): RegExpExecArray;

    private native matchImpl(pattern: String, flags: String, str: String, patternLength: number, strLength: number, lastIndex: number, hasSlashU: boolean): RegExpMatchArray;

    internal exec(str: String, index: number): RegExpExecArray | null {
        const hasSlashU = str.contains("\\u", 0) || this.pattern_.contains("\\u", 0)
        const res = this.execImpl(this.pattern_, this.flags_, str, this.pattern_.length, str.length, index, hasSlashU)
        if (res.isCorrect) {
            res.input = str
            RegExp.lastMatch_ = ""
            RegExp.lastParen_ = ""
            res.postExecProcessing(res, str, res.index, this.hasIndices)
            return res
        } else {
            return null
        }
    }

    /**
     * Executes a search for a match in a specified string and returns a result array
     *
     * @param str the string against which to match the regular expression
     *
     * @returns RegExp result
     *
     * @see RegExpExecArray
     */
    public exec(str: String): RegExpExecArray | null {
        return this.exec(str, this.lastIndex)
    }

    internal matchInternal(str: String, index: number): RegExpMatchArray | null {
        const hasSlashU = str.contains("\\u", 0) || this.pattern_.contains("\\u", 0)
        const res = this.matchImpl(this.pattern_, this.flags_, str, this.pattern_.length, str.length, index, hasSlashU)
        if (res.isCorrect) {
            res.input = str
            res.postExecProcessing(res, str, res.index!, this.hasIndices)
            return res
        } else {
            return null
        }
    }

    internal matchInternal(str: String): RegExpMatchArray | null {
        return this.matchInternal(str, this.lastIndex)
    }

    public search(str: String): number {
        let previousLastIndex = this.lastIndex
        this.lastIndex = 0
        let result = this.exec(str)
        this.lastIndex = previousLastIndex
        if (result == null) {
            return -1
        } else {
            return result.index
        }
    }

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
                let thisIndex = this.lastIndex
                let nextIndex = RegExp.advanceStringIndex(str, thisIndex, this.unicode)
                this.lastIndex = nextIndex
            }
        }
        let lengthS = str.getLength()
        let accumulatedResult = ""
        let nextSourcePosition = 0
        for (let i = 0; i < results.length; ++i) {
            let result = results.at(i)!;
            let nCaptures = result.length;
            let matched = result.$_get(0)!;
            let matchLength = matched.getLength();
            let position = max(min(result.index, lengthS), 0).toInt();
            let captures = new containers.UndefinableStringArray(nCaptures);
            for (let n = 1; n < nCaptures; n++) {
                let capN = result.$_get(n);
                captures.pushBack(capN)
            }
            let replacement = String.getSubstitution(matched, str, position as int, captures.toArray(), result.groups, replaceValue)
            if (position >= nextSourcePosition) {
                accumulatedResult += str.substring(nextSourcePosition, position) + replacement
                nextSourcePosition = position + matchLength
            }
        }
        if (nextSourcePosition >= lengthS) {
            return accumulatedResult
        }
        return accumulatedResult + str.substring(nextSourcePosition)
    }

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
                let thisIndex = this.lastIndex
                let nextIndex = RegExp.advanceStringIndex(str, thisIndex, this.unicode)
                this.lastIndex = nextIndex
            }
        }
        let lengthS = str.getLength()
        let accumulatedResult = ""
        let nextSourcePosition = 0
        for (let i = 0; i < results.length; ++i) {
            let result = results.at(i)!;
            let nCaptures = result.length;
            let matched = result.$_get(0)!;
            let matchLength = matched.getLength();
            let position = (max(min(result.index, lengthS), 0)).toInt();
            let args = new containers.UndefinableObjectArray(nCaptures);
            for (let n = 1; n < nCaptures; n++) {
                let capN = result.$_get(n);
                args.pushBack(capN)
            }
            let namedCaptures: NullishType = /*result.groups*/ undefined;
            args.pushBack(position.toDouble());
            args.pushBack(str)
            if (namedCaptures != undefined) {
                args.pushBack(namedCaptures)
            }
            let replacement = replacer(matched, args.toArray());
            if (position >= nextSourcePosition) {
                accumulatedResult += str.substring(nextSourcePosition, position) + replacement
                nextSourcePosition = position + matchLength
            }
        }
        if (nextSourcePosition >= lengthS) {
            return accumulatedResult
        }
        return accumulatedResult + str.substring(nextSourcePosition)
    }

    public split(str: String, limit: Number | undefined): String[] {

        let unicodeMatching = this.unicode
        let newFlags = this.sticky ? this.flags : this.flags + "y";
        let splitter = new RegExp(this.pattern_, newFlags)
        let lim: long;
        if (limit == undefined) {
            lim = (1 << 32) -1
        } else if (limit == 0) {
            return new String[0]
        } else {
            lim = limit.toLong()
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
        for (let lastEnd = 0; lastEnd < size;) {
            splitter.lastIndex = lastEnd
            let z = splitter.exec(str)
            if (z == null) {
                lastEnd = RegExp.advanceStringIndex(str, lastEnd, unicodeMatching)
            } else {
                let separatorRight = (splitter.lastIndex < size) ? splitter.lastIndex.toInt() : size
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
        return splittedStrings.toArray() as String[]
    }

    public match(str: String): RegExpMatchArray | null {
        if (!this.global) {
            return this.matchInternal(str)
        }
        this.lastIndex = 0;
        let matches = new containers.UndefinableStringArray();
        let indices = new Array<Array<number>>();
        let isUndefined = new Array<boolean>();
        let n = 0
        while (true) {
            let result = this.matchInternal(str)
            if (result == null) {
                if (n == 0) {
                    return null
                }
                return new RegExpMatchArray(-1, "", matches.toArray(), indices, isUndefined)
            }
            else {
                let matchStr = result.$_get(0)
                matches.pushBack(matchStr)
                indices = indices.concat(result.indices)
                isUndefined = isUndefined.concat(result.isUndefined)
                if (matchStr == "") {
                    this.lastIndex = RegExp.advanceStringIndex(str, this.lastIndex, this.unicode)
                }
                n++
            }
        }
    }

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
     * @param str the string against which to match the regular expression
     *
     * @returns true if match was found
     */
    public test(str: String): boolean {
        let res = this.exec(str)
        if (!res) {
            return false
        }
        return res.isCorrect
    }

    /**
     * Returns a string representing the given object
     *
     * @returns a string representing the given object
     */
    public override toString(): String {
        return "/" + this.source + "/" + this.flags_
    }

    /**
     * Returns next index from a passed one
     *
     * @param s
     *
     * @param index start position
     *
     * @param unicode true if unicode is used
     *
     * @returns new index
     */
    public static advanceStringIndex(s: String, index: int, unicode: boolean): int {
        if (!unicode) {
            return index + 1
        }
        let length = s.getLength();
        if (index + 1 >= length) {
            return index + 1
        }
        return index + s.codePointCount(index, index + 1);
    }

    /**
     * Returns next index from a passed one
     *
     * @param s
     *
     * @param index start position
     *
     * @param unicode true if unicode is used
     *
     * @returns new index
     */
    public static advanceStringIndex(s: String, index: number, unicode: boolean): number {
        return RegExp.advanceStringIndex(s, index.toInt(), unicode)
    }

    public static $_invoke(pattern: RegExp | String, flags?: String) : RegExp {
        return new RegExp(pattern, flags)
    }

    public static $_invoke(pattern: RegExp | String) : RegExp {
        return new RegExp(pattern)
    }

    public static $_invoke(pattern: String, flags?: String) : RegExp {
        return new RegExp(pattern, flags)
    }

    private static escapePattern(pattern: String): String {
        if (pattern == "") {
            return "(?:)"
        }
        let s = pattern.replaceAll("/", "\\/")
        return s.replaceAll("\\", "\\")
    }

    /**
     * Inits regexp from pattern and flags
     *
     * @param pattern description of a pattern
     *
     * @param flags description of flags to be used
     */
    private init(pattern: String, flags?: String) {
        this.regexp_parser(pattern)
        this.pattern_ = pattern
        this.flags_ = flags === undefined ? "" : flags!
        this.source_ = RegExp.escapePattern(pattern)
        this.flags_ = Array.from(this.flags_.split("")).sort().join("")
    }

    /**
     * Inits regexp from RegExp. It uses regexp's flags if flags aren't provided
     *
     * @param regexp other regexp
     *
     * @param flags description of flags to be used
     */
    private init(regexp: RegExp, flags?: String) {
        this.init(regexp.pattern_, flags === undefined ? regexp.flags : flags!)
    }

    internal regexp_parser(pattern: String): void {
        let err: String | undefined = RegExp.parse(pattern)
        if (err != undefined) {
            throw new SyntaxError(err as String)
        }
    }
}
