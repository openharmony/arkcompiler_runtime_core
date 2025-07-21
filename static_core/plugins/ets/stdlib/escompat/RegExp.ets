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
export class RegExpExecArray /*extends Array<String>*/ {
    /** true if match present */
    readonly isCorrect: boolean
    /** result itself */
    private result_: FixedArray<String>
    /** The 0-based index of the match in the string */
    public index: number
    /** The original string that was matched against */
    public input: String

    // NOTE(shumilov-petr): Make nullable when #14813 will be fixed
    readonly indices: FixedArray<FixedArray<number>>;

    /** named groups names and indeces as JSON */
    private groupsRaw_: String = ""

    /** named groups names and indeces */
    public groups: RegExpGroupsContainer | undefined = undefined

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
    public $_get(index: int): String {
        return this.result_[index]
    }

    /**
     * Returns result string by index
     *
     * @param index
     *
     * @returns resulting string
     */
    public $_get(index: number): String {
        return this.$_get(Double.toInt(index));
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
        return this.$_set(Double.toInt(index), val);
    }

    get length(): int {
        return this.result_.length as int;
    }

    public get result(): String[] {
        return Array.from<String>(this.result_)
    }

    /**
     * Parses groupsRaw_ to groups
     */
    internal updateGroups(): void {
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
                if (endIndex > this.input.length || index >= endIndex) {
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

    /**
     * Appends one element to result
     *
     * @param string to add
     */
    public push(newElem: string): void {
        // NOTE(kparshukov): improve speed when T[] <=> Arrat<T>
        const copy : FixedArray<String> = new String[this.result_.length + 1]
        for (let i = 0; i < this.result_.length; i++) {
            copy[i] = this.result_[i]
        }
        this.result_ = copy
        this.result_[this.result_.length as int - 1] = newElem
    }

    /**
     * Removes and returns the last match result in the array.
     * Simulates the behavior of Array.prototype.pop().
     *
     * @returns The last matched string if present; otherwise, undefined.
     */
    public pop(): String | undefined {
        if (this.result_.length === 0) {
            return undefined;
        }

        const lastIndex = this.result_.length - 1;
        const last = this.result_[lastIndex];

        let newResult: FixedArray<String> = new String[lastIndex];
        for (let i = 0; i < lastIndex; i++) {
            newResult[i] = this.result_[i];
        }

        this.result_ = newResult;

        return last;
    }

    constructor(index: int, input: String, result: FixedArray<String>, indices: FixedArray<FixedArray<number>>) {
        this.isCorrect = true
        this.index = index
        this.input = input
        this.result_ = result
        this.indices = indices
    }

    constructor(index: number, input: String, result: FixedArray<String>) {
        this.isCorrect = true
        this.index = index
        this.input = input
        this.result_ = result
        this.indices = []
    }

    constructor(index: number, input: String, result: FixedArray<String>, indices: FixedArray<FixedArray<number>>) {
        this(index.toInt(), input, result, indices)
    }
}

// NOTE(kparshukov): should be splitted in #24868
export type RegExpMatchArray = RegExpExecArray

class GlobalRegExpStringIterator implements IterableIterator<RegExpMatchArray> {
    private regexp: RegExp
    private string: String

    constructor(regexp: RegExp, string: String) {
        this.regexp = regexp
        this.string = string
    }

    override next(): IteratorResult<RegExpMatchArray> {
        let match = this.regexp.exec(this.string)
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
        let match = this.regexp.exec(this.string)
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
    public lastIndex: number
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

    private groupNames: String[] = []
    private buffer: int[] = new Array<int>()

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

    private native execImpl(pattern: String, flags: String, str: String, hasSlashU: boolean): RegExpExecArray;

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
        let hasSlashU = str.contains("\\u", 0) || this.pattern_.contains("\\u", 0)
        const res = this.execImpl(this.pattern_, this.flags_, str, hasSlashU)
        if (res.isCorrect) {
            res.updateGroups()
            return res
        } else {
            return null
        }
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
            let matched = result.$_get(0);
            let matchLength = matched.getLength();
            let position = max(min(result.index, lengthS), 0).toInt();
            let captures = new UndefinableStringArray(nCaptures);
            for (let n = 1; n < nCaptures; n++) {
                let capN = result.$_get(n);
                captures.pushBack(capN)
            }
            let namedCaptures: Object | undefined = result.groups;
            let replacement = String.getSubstitution(matched, str, position as int, captures.toArray(), namedCaptures, replaceValue)
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
            let matched = result.$_get(0);
            let matchLength = matched.getLength();
            let position = (max(min(result.index, lengthS), 0)).toInt();
            let args = new UndefinableObjectArray(nCaptures);
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
        let splittedStrings = new UndefinableStringArray()
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
                        let nextCapture = z.$_get(i);
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
            return this.exec(str)
        }
        this.lastIndex = 0;
        let matches = new UndefinableStringArray();
        let n = 0
        while (true) {
            let result = this.exec(str)
            if (result == null) {
                if (n == 0) {
                    return null
                }
                return new RegExpMatchArray(-1, "", matches.toFixedArray())
            }
            else {
                let matchStr = result.$_get(0)
                matches.pushBack(matchStr)
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
        let hasSlashU = str.contains("\\u", 0) || this.pattern_.contains("\\u", 0)
        return this.execImpl(this.pattern_, this.flags_, str, hasSlashU).isCorrect
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
