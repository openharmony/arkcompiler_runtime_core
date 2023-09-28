/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

package escompat;

// TODO(kirill-mitkin): change to inner class later
/**
 * Regular expression result descriptor
 */
export class RegExpExecResult extends Object {
    /** true if match present */
    readonly isCorrect: boolean;
    /** The 0-based index of the match in the string */
    readonly index: int;
    /** The original string that was matched against */
    readonly input: String;
    /** result itself */
    readonly result: String[];
    // TODO(kirill-mitkin): add indices and groups fields

    /**
     * Returns result string by index
     *
     * @param index
     *
     * @returns resulting string
     */
    public get(index: int): String {
        return this.result[index];
    }

    /**
     * Returns result string by index
     *
     * @param index
     *
     * @returns resulting string
     */
    public get(index: number): String {
        return this.get(index as int);
    }

    constructor(isCorrect: boolean, index: int, input: String, result: String[]) {
        this.isCorrect = isCorrect;
        this.index = index;
        this.input = input;
        this.result = result;
    }

    constructor(isCorrect: boolean, index: number, input: String, result: String[]) {
        this(isCorrect, index as int, input, result)
    }

    public override equals(other: Object|null): boolean {
        if (!(other instanceof RegExpExecResult)) {
            return false;
        }
        let o: RegExpExecResult = other as RegExpExecResult;

        let res =
            (this.isCorrect == o.isCorrect) &&
            (this.index == this.index) &&
            (this.input.compareTo(o.input) == 0) &&
            (this.result.length == o.result.length);

        if (!res) {
            return false;
        }

        for (let i: int = 0; i < this.result.length; i++) {
            res &= (this.result[i].compareTo(o.result[i]) == 0);
            if (!res) {
                return false;
            }
        }

        return true;
    }
}

/**
 * Regular expression
 */
export class RegExp extends Object {
    /**
     * The string against which a regular expression is matched
     */
    readonly pattern: String;

    /** Flags */
    readonly flags: String;

    // TODO(kirill-mitkin): change to properties
    /**
     * Has the value true if the regular expression should be tested against
     * all possible matches in a string
     */
    readonly global: boolean;

    /**
     * Has the value true if the dot special character (.) should additionally match
     * the line terminator characters in a string
     */
    readonly dotAll: boolean;

    /**
     * Has the value true if the result of a regular expression match should contain
     * the start and end indices of the substrings of each capture group
     */
    readonly hasIndices: boolean;

    /**
     * Has the value true if case should be ignored while attempting a match in a string
     */
    readonly ignoreCase: boolean;

    /**
     * Has the value true if a multiline input string should be treated as multiple lines
     */
    readonly multiline: boolean;

    /**
     * Has the value true if the regex attempts to match the target string only
     * from the index indicated by the lastIndex property
     */
    readonly sticky: boolean;

    /**
     * Has the value true if 'u' flag is used
     */
    readonly unicode: boolean;

    /**
     * Has the value true if 'v' flag is used
     */
    readonly unicodeSets: boolean;

    /**
     * The index at which to start the next match
     */
    lastIndex: int;

    /**
     * Returns a string containing the source text of this regular expression
     */
    readonly source: String;

    /**
     * Returns the string against which a regular expression is matched
     */
    readonly static input: String;

    /**
     * An alias for input
     */
    readonly static $_: String;

    /**
     * Returns the last matched substring
     */
    readonly static lastMatch: String;

    /**
     * An alias for lastMatch
     */
    // readonly static $&: String;

    /**
     * Returns the last parenthesized substring match, if any
     */
    readonly static lastParen: String

    /**
     * An alias for lastParen
     */
    // readonly static $+: String

    /**
     * Returns the substring preceding the most recent match
     */
    readonly static leftContext: String

    /**
     * An alias for leftContext
     */
    // readonly static $`: String

    /**
     * Returns the substring following the most recent match
     */
    readonly static rightContext: String

    /**
     * An alias for rightContext
     */
    // readonly static $': String

    /**
     * Static accessor properties return parenthesized substring matches
     */
    readonly static $1: String, $2: String, $3: String, $4: String, $5: String, $6: String, $7: String, $8: String, $9: String;

    /**
     * Constructs a new RegExp using pattern
     *
     * @param pattern description of a pattern
     */
    public constructor(pattern: String) {
        this(pattern, "");
    }

    /**
     * Constructs a new RegExp using pattern and flags
     *
     * @param pattern description of a pattern
     *
     * @param flags description of flags to be used
     */
    public constructor(pattern: String, flags: String) {
        this.pattern = pattern;
        this.flags = flags;
        this.source = RegExp.escapePattern(pattern);
        this.initializeFlags();
        this.compile();
    }

    /**
     * Compiles a regular expression
     */
    private native compile();

    /**
     * Recompiles a regular expression with new source and flags after the RegExp object has already been created
     *
     * @param pattern the text of the regular expression
     *
     * @param flags any combination of flag values
     */
    public compile(pattern : String, flags: String) {
        this.pattern = pattern;
        this.flags = flags;
        this.source = RegExp.escapePattern(pattern);
        this.initializeFlags();
        this.compile();
    }

    /**
     * Executes a search for a match in a specified string and returns a result array
     *
     * @param str the string against which to match the regular expression
     *
     * @returns RegExp result
     *
     * @see RegExpExecResult
     */
    public native exec(str: String): RegExpExecResult;

    /**
     * Executes a search for a match between a regular expression and specified string
     *
     * @param str the string against which to match the regular expression
     *
     * @returns true if match was found
     */
    public test(str: String): boolean {
        return this.exec(str).isCorrect;
    }

    /**
     * Returns a string representing the given object
     *
     * @returns a string representing the given object
     */
    public override toString(): String {
        return "/" + this.source + "/" + this.flags;
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
            return index + 1;
        }
        let length = s.length();
        if (index + 1 >= length) {
            return index + 1;
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
        return RegExp.advanceStringIndex(s, index as int, unicode)
    }

    private initializeFlags() {
        if (this.flags.isEmpty()) {
            return;
        }
        this.global = this.flags.contains("g", 0);
        this.hasIndices = this.flags.contains("d", 0);
        this.ignoreCase = this.flags.contains("i", 0);
        this.multiline = this.flags.contains("m", 0);
        this.unicode = this.flags.contains("u", 0);
        this.dotAll = this.flags.contains("s", 0);
        this.sticky = this.flags.contains("y", 0);
        this.unicodeSets = this.flags.contains("v", 0);
    }

    private static escapePattern(pattern: String): String {
        if (pattern == "") {
            return "(?:)";
        }
        let s = pattern.replaceAll("/", "\\/");
        return s.replaceAll("\\", "\\");
    }

    private groupNames: String[];
    private buffer: int[];
}
