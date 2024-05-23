/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

function main(): void {
    let str1: String = new String();
    assert str1.length == 0: "String is not empty";
    assert str1.isEmpty(): "String is not empty";

    let chars: char[] = [c'a', c'B', c'C', c'd', c'e', c'g', c'X', c'Y', c'z'];
    let str2: String = new String(chars);  // "aBCdegXYz"
    assert str2.charAt(0) == c'a': "Wrong str2.charAt(0)";
    assert str2.charAt(1) == c'B': "Wrong str2.charAt(1)";
    assert str2.charAt(2) == c'C': "Wrong str2.charAt(2)";
    assert str2.charAt(8) == c'z': "Wrong str2.charAt(8)";
    assert str2.charAt(8) != c'Z': "Wrong str2.charAt(8): invalid character case";

    let cb: StringBuilder = new StringBuilder("abc123");
    let str3: String = "abc123";
    assert str3.contentEquals(cb): "contentEquals is wrong";

    str3 = "hello";
    chars = str3.getChars();
    assert chars.length == 5: "Chars length must be 5";
    assert chars[0] == c'h': "First letter must be `h`";
    assert chars[1] == c'e': "Second letter must be `e`";
    assert chars[2] == c'l': "Third letter must be `l`";
    assert chars[3] == c'l': "Fourth letter must be `l`";
    assert chars[4] == c'o': "Fifth letter must be `o`";

    str2 = "bytesStr";
    let byteChars: byte[] = str2.getBytes(0, str2.length as int);
    assert byteChars[0] == 98: "First byte must be 98";
    assert byteChars[1] == 121: "Second byte must be 121";
    assert byteChars[2] == 116: "Third byte must be 116";
    assert byteChars[3] == 101: "Fourth byte must be 101";
    assert byteChars[4] == 115: "Fifth byte must be 115";

    assert str3.startsWith('h') == true: "String must be started with `h`";
    assert str3.startsWith("hel") == true: "String must be started with `hel`";
    assert str3.startsWith("hel", 0) == true: "String must be started with `hel`";
    assert str3.startsWith("ello", 1) == true: "String must be started with `hel`";

    assert str3.endsWith("o") == true: "String must be ended with `o`";
    assert str3.endsWith("lo") == true: "String must be ended with `lo`";
    assert str3.endsWith("hello") == true: "String must be ended with `hello`";

    assert str3.indexOf(c'h') == 0: "IndexOf 'h' must be 0";
    assert str3.indexOf(c'e') == 1: "IndexOf 'e' must be 1";
    assert str3.indexOf(c'l') == 2: "IndexOf first 'l' must be 2";
    assert str3.indexOf(c'l', 3) == 3: "IndexOf second 'l' must be 3";
    assert str3.indexOf(c'o', 3) == 4: "IndexOf 'o' must be 4";
    assert str3.indexOf(c'o') == 4: "IndexOf 'o' must be 4";

    assert str3.indexOf("el") == 1: "IndexOf 'el' must be 1";
    assert str3.indexOf("lo") == 3: "IndexOf 'lo' must be 3";

    assert str3.lastIndexOf("l") == 3: "LastIndexOf 'l' must be 3";
    assert str3.lastIndexOf(c'l', 2) == 2: "LastIndexOf 'l' from idx 2 must be 2";

    assert str3.substring(0).equals("hello" as Object): "Substring from idx 0 must be 'hello'";
    assert str3.substring(3).equals("lo" as Object): "Substring from idx 3 must be 'lo'";
    assert str3.substring(0, 3).equals("hel" as Object): "Substring from idx 0 to 3 must be 'hel'";
    assert str3.substring(1, 3).equals("el" as Object): "Substring from idx 1 to 3 must be 'el'";
    assert str3.substring(4, 5).equals("o" as Object): "Substring from idx 4 to 5 must be 'o'";
    assert str3.substring(4, 4).equals("" as Object): "Substring from idx 4 to 4 must be empty string";
    assert !str3.substring(3, 5).equals("o" as Object): "Substring from idx 3 to 4 must not be 'o'";

    assert str3.concat(", world!").equals("hello, world!" as Object): "Strings must be equal after concat";

    assert str3.replaceChar(c'e', c'a').equals("hallo" as Object): "Strings must be equal after replaceChar";
    assert str3.replaceChar(c'l', c't').equals("hetto" as Object): "Strings must be equal after replaceChar";
    assert str3.replaceChar(c'm', c'v').equals("hello" as Object): "Strings must be equal after replaceChar";

    assert str3.contains("hel", 0): "String must contain 'hel'";
    assert !str3.contains("hel", 1): "String must not contain 'hel' from idx 1";
    assert !str3.contains("hal", 0): "String must not contain 'hal'";

    str1 = String.join(["AbCd", "Xtt", "G"], ":", "|pRef|", "|SUF|");
    assert str1.equals("|pRef|AbCd|SUF|:|pRef|Xtt|SUF|:|pRef|G|SUF|" as Object): "Strings must be equal after join";

    assert  str1.toLowerCase().equals("|pref|abcd|suf|:|pref|xtt|suf|:|pref|g|suf|" as Object):
            "Strings must be equal after toLowerCase";
    assert  str1.toUpperCase().equals("|PREF|ABCD|SUF|:|PREF|XTT|SUF|:|PREF|G|SUF|" as Object):
            "Strings must be equal after toUpperCase";

    str1 = "Sky";
    assert str1.padLeft(c'.', 0).equals("Sky" as Object): "String must be equal 'Sky'";
    assert str1.padLeft(c'.', 3).equals("Sky" as Object): "String must be equal 'Sky'";
    assert str1.padLeft(c'.', 4).equals(".Sky" as Object): "String must be equal '.Sky'";
    assert str1.padLeft(c'.', 6).equals("...Sky" as Object): "String must be equal '...Sky'";
    assert str1.padRight(c'-', 1).equals("Sky" as Object): "String must be equal 'Sky'";
    assert str1.padRight(c'-', 5).equals("Sky--" as Object): "String must be equal 'Sky--'";

    str1 = "A"
    try {
        assert str1.repeat(0).equals("" as Object): "String must be empty after repeat";
        assert str1.repeat(1).equals("A" as Object): "String must be 'A' after repeat";
        assert str1.repeat(4).equals("AAAA" as Object): "String must be 'AAAA' after repeat";
    } catch(e) {
        assert false : "Unexpected exception in String.repeat";
    }

    let b : boolean = true;
    str1 = "ABCD : " + b;
    assert str1.equals("ABCD : true" as Object): "String must be 'ABCD : true'";

    str1 = "ä";
    assert str1.localeCompare("z", "de") == -1: "Must be -1. In German, ä sorts before z";
    assert str1.localeCompare("z", "sv") == 1: "Must be 1. In Swedish, ä sorts after z";

    str1 = "It can't be - there must be a gap in between!";
    let arr: String[] = str1.split(" ", -5);
    assert arr.length == 11: "Length must be 11";
    assert arr[0].equals("It"): "First substring must be `It`";
    assert arr[1].equals("can't"): "Second substring must be `can't`";
    assert arr[2].equals("be"): "Third substring must be `be`";
    assert arr[3].equals("-"): "Third substring must be `-`";
    assert arr[4].equals("there"): "Third substring must be `there`";
    assert arr[5].equals("must"): "Third substring must be `must`";
    assert arr[6].equals("be"): "Third substring must be `be`";
    assert arr[7].equals("a"): "Third substring must be `a`";
    assert arr[8].equals("gap"): "Third substring must be `gap`";
    arr = str1.split(" ", 3);
    assert arr.length == 3: "Length must be 3";

    let str4: String = "Example string";
    assert str4[0] == c'E': "First character must be `E`";
    assert str4[8] == c's': "Eighth character must be `s`";
    assert str4[str4.length - 1] == c'g': "Last character must be `g`";

    try {
        let failedCase = str4[-1];
        assert false
    } catch(err) {
        assert err instanceof StringIndexOutOfBoundsError: "It should be instance of StringIndexOutOfBoundsError"
    }

    try {
        let failedCase = str4[str4.length];
        assert false
    } catch(err) {
        assert err instanceof StringIndexOutOfBoundsError: "It should be instance of StringIndexOutOfBoundsError"
    }

    let str5: String = "函数式编程";
    assert str5[0] == c'函': "First character must be `函`";
}
