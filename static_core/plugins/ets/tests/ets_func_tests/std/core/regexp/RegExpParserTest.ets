/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

function testRegExpParserCase1() {
    const str1: string = '😀abc';
    const regex1 = new RegExp('\\p{Emoji}', 'u');
    const result1: number = str1.search(regex1);
    arktest.assertEQ(result1, 0);

    const str2: string = 'a𐐀b𐐁'
    const regex2 = new RegExp('([a-z\\u{10400}-\\u{1041F}])', 'gu');
    const result2 = Array.from(str2.matchAll(regex2))

    let str3: string = '你好';
    let regex3 = new RegExp('\\p{L}', 'u');
    arktest.assertEQ(regex3.test(str3), true);
    arktest.assertEQ(regex3.test('123'), false);

    let str4: string = 'Hello 𐐷 World';
    let regex4 = new RegExp('\\p{Script=Deseret}', 'gu');
    let result4 = str4.match(regex4);
    arktest.assertEQ(result4![0], ['𐐷'][0]);

    const str5 = 'hello\u0007world';
    const regex5 = new RegExp('[\x00-\x1F]', 'g');
    const result5 = str5.match(regex5);
    arktest.assertEQ(result5![0], ['\u0007'][0]);
}

function testRegExpParserCase2() {
    arktest.expectThrow(() => { new RegExp('a𐐀b𐐁','z'); }, expectedSyntaxError)
}

function testRegExpParserCase3() {
    arktest.expectThrow(() => { new RegExp("(","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp(")","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("{","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("}","u"); }, expectedSyntaxError)
}

function testRegExpParserCase4() {
    arktest.expectThrow(() => { new RegExp("[\\d-a]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\D-a]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\s-a]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\S-a]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\w-a]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[a-\\d]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[a-\\D]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[a-\\s]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[a-\\S]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[a-\\w]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[a-\\W]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\d-\\d]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\D-\\D]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\s-\\s]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\S-\\S]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\w-\\w]","u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\W-\\W]","u"); }, expectedSyntaxError)
}

function testRegExpParserCase5() {
    let isSyntaxCharacter = (c: string): boolean =>{
        switch (c) {
            case "^":
            case "$":
            case "\\":
            case ".":
            case "*":
            case "+":
            case "?":
            case "(":
            case ")":
            case "[":
            case "]":
            case "{":
            case "}":
            case "|":
                return true;
            default:
                return false;
        }
    }
    let isAlphaDight = (c: string): boolean => {
        return ("0" <= c && c<= "9") || ("A" <= c && c<= "Z") || ("a" <= c && c<= "z")
    }
    for(let cu = 0x00; cu <= 0x7f; ++cu){
        let s = String.fromCharCode(cu);
        if(!isAlphaDight(s) && !isSyntaxCharacter(s) && s !== "/"){
            arktest.expectThrow(() => { new RegExp("\\"+s,"u"); }, expectedSyntaxError)
        }
    }
    for(let cu = 0x00; cu <= 0x7f; ++cu){
        let s = String.fromCharCode(cu);
        if(!isAlphaDight(s) && !isSyntaxCharacter(s) && s !== "/" && s !== "-"){
            arktest.expectThrow(() => { new RegExp("[\\"+s+"]","u"); }, expectedSyntaxError)
        }
    }
}

function testRegExpParserCase6() {
    let isValidAlphaEscapeInAtom = (s: string): boolean =>{
        switch (s) {
            case "b":
            case "B":
            case "f":
            case "n":
            case "r":
            case "t":
            case "v":
            case "d":
            case "D":
            case "s":
            case "S":
            case "w":
            case "W":
                return true;
            default:
                return false;
        }
    }
    let isValidAlphaEscapeInClass = (s: string): boolean =>{
        switch (s) {
            case "b":
            case "f":
            case "n":
            case "r":
            case "t":
            case "v":
            case "d":
            case "D":
            case "s":
            case "S":
            case "w":
            case "W":
                return true;
            default:
                return false;
        }
    }
    const uppercaseLetters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    const lowercaseLetters = 'abcdefghijklmnopqrstuvwxyz';
    for(let s of uppercaseLetters) {
        if(!isValidAlphaEscapeInAtom(s)) {
            arktest.expectThrow(() => { new RegExp("\\"+s,"u"); }, expectedSyntaxError)
        }
        if (!isValidAlphaEscapeInClass(s)){
            arktest.expectThrow(() => { new RegExp("[\\" + s + "]", "u"); }, expectedSyntaxError)
        }
    }
    for(let s of lowercaseLetters){
        if(!isValidAlphaEscapeInAtom(s)){
            arktest.expectThrow(() => { new RegExp("\\" + s, "u"); }, expectedSyntaxError)
        }
        if (!isValidAlphaEscapeInClass(s)){
            arktest.expectThrow(() => { new RegExp("[\\" + s + "]", "u"); }, expectedSyntaxError)
        }
    }
}

function testRegExpParserCase7() {
    let isAlpha = (c: string): boolean => {
        return ("A" <= c && c <= "Z") || ("a" <= c && c <= "z");
    }
    arktest.expectThrow(() => { new RegExp("\\c", "u"); }, expectedSyntaxError)
    for (let cu = 0x00; cu <= 0x7f; ++cu){
        let s = String.fromCharCode(cu);
        if(!isAlpha(s)) {
            arktest.expectThrow(() => { new RegExp("\\c" + s, "u"); }, expectedSyntaxError)
        }
    }
    arktest.expectThrow(() => { new RegExp("[\\c]", "u"); }, expectedSyntaxError)
    for (let cu = 0x00; cu <= 0x7f; ++cu){
        let s = String.fromCharCode(cu);
        if(!isAlpha(s)) {
            arktest.expectThrow(() => { new RegExp("[\\c" + s + "]", "u"); }, expectedSyntaxError)
        }
    }
}

function testRegExpParserCase8() {
    arktest.expectThrow(() => { new RegExp("\\u", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\u1", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\u12", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\u123", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\u{", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\u{}", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\u{1", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\u{12", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\u{123", "u"); }, expectedSyntaxError)

    //Incomplete RegExpUnicodeEscapeSequence in ClassEscape not parsed as IdentityEscape.
    //ClassEscape[U] :: CharacterEscape[?U]
    //CharacterEscape[U] :: RegExpUnicodeEscapeSequence[?U]
    arktest.expectThrow(() => { new RegExp("[\\u]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\u1]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\u12]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\u123]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\u{]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\u{}]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\u{1]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\u{123]", "u"); }, expectedSyntaxError)
}

function testRegExpParserCase9() {
    arktest.expectThrow(() => { new RegExp("\\x", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\x1", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\x]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\x1]", "u"); }, expectedSyntaxError)
}

function testRegExpParserCase10() {
    // Incomplete quantifier with atom.
    arktest.expectThrow(() => { new RegExp("a{", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("a{1", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("a{1,", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("a{1,2", "u"); }, expectedSyntaxError)
    // // Incomplete quantifier without atom.
    arktest.expectThrow(() => { new RegExp("{", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("{1", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("{1,", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("{1,2", "u"); }, expectedSyntaxError)
}

function testRegExpParserCase11() {
    arktest.expectThrow(() => { new RegExp("\\1", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\2", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\3", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\4", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\5", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\6", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\7", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\8", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\9", "u"); }, expectedSyntaxError)

    // DecimalEscape without leading 0 in ClassEscape
    // ClassEscape[U] :: DecimalEscape
    // DecimalEscaoe :: DecimalIntegerLiteral [lookahead /= DecimalDigit]
    arktest.expectThrow(() => { new RegExp("[\\1]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\2]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\3]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\4]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\5]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\6]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\7]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\8]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\9]", "u"); }, expectedSyntaxError)

    // DecimalEscape with leading 0 in AtomEscape.
    // Atom[U] :: DecimalEscape
    // DecimalEscaoe :: DecimalIntegerLiteral [lookahead /= DecimalDigit]
    arktest.expectThrow(() => { new RegExp("\\00", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\01", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\02", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\03", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\04", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\05", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\06", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\07", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\08", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("\\09", "u"); }, expectedSyntaxError)

    // DecimalEscape with leading 0 in ClassEscape.
    // ClassEscape[U] :: DecimalEscape
    // DecimalEscaoe :: DecimalIntegerLiteral [lookahead /= DecimalDigit]
    arktest.expectThrow(() => { new RegExp("[\\00]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\01]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\02]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\03]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\04]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\05]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\06]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\07]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\08]", "u"); }, expectedSyntaxError)
    arktest.expectThrow(() => { new RegExp("[\\09]", "u"); }, expectedSyntaxError)
}

function main(): int {
    let suite = new arktest.ArkTestsuite("parse test");
    suite.addTest("test1",testRegExpParserCase1);
    suite.addTest("test2",testRegExpParserCase2);
    suite.addTest("test3",testRegExpParserCase3);
    suite.addTest("test4",testRegExpParserCase4);
    suite.addTest("test5",testRegExpParserCase5);
    suite.addTest("test6",testRegExpParserCase6);
    suite.addTest("test7",testRegExpParserCase7);
    suite.addTest("test8",testRegExpParserCase8);
    suite.addTest("test9",testRegExpParserCase9);
    suite.addTest("test10",testRegExpParserCase10);
    suite.addTest("test11",testRegExpParserCase11);
    return suite.run();
}

// helper
const expectedSyntaxError = (e: Error): boolean => {
    return (e instanceof SyntaxError)
}
