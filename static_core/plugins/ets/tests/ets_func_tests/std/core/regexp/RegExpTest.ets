/**
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

function main(): int {
    const suite = new arktest.ArkTestsuite("RegExp Test");

    // Exec and replacement behavior
    suite.addTest('Alternation exec patterns', testAlternationExecPatterns);
    suite.addTest('Emoji replacement modes', testEmojiReplacementModes);
    suite.addTest('String replacement variants', testStringReplacementVariants);
    suite.addTest('Literal replace fast path', testLiteralReplaceFastPath);
    suite.addTest('ReplaceAll capture groups', testReplaceAllGroupCallbacks);

    // Core pattern coverage
    suite.addTest('Basic literal and URL patterns', testBasicLiteralAndUrlPatterns);
    suite.addTest('Unicode alphabet ranges', testUnicodeAlphabetRanges);
    suite.addTest('Unicode alphanumeric ranges', testUnicodeAlphaNumericRanges);
    suite.addTest('Numeric and currency patterns', testNumericAndCurrencyPatterns);
    suite.addTest('Base64 alphabet pattern', testBase64AlphabetPattern);

    // Cache and state-sensitive behavior
    suite.addTest('Cache-sensitive replace and test loops', testCacheSensitiveReplaceAndTestLoops);
    suite.addTest('LastIndex-aware exec behavior', testLastIndexAwareExecBehavior);
    suite.addTest('Replace caching preserves lastIndex', testReplaceCachingPreservesLastIndex);
    suite.addTest('Split, search, and match state behavior', testSplitSearchAndMatchStateBehavior);

    // Complex strings and edge cases
    suite.addTest('Split and markup extraction patterns', testSplitAndMarkupExtractionPatterns);
    suite.addTest('Empty alternative exec behavior', testEmptyAlternativeAndExecBehavior);
    suite.addTest('Regex flag reporting', testRegexFlagReporting);
    suite.addTest('Escaped bracket patterns', testEscapedBracketPatterns);
    suite.addTest('UTF-8 replacement branches', testUtf8ReplacementBranches);
    suite.addTest('Large pattern replacement', testLargePatternReplacement);
    suite.addTest('Optional group exec', testOptionalGroupExec);

    // Unicode flag behavior
    suite.addTest('DotAll unicode flags', testRegexDotAllUnicodeFlags);
    suite.addTest('Unicode flag only', testRegexUnicodeFlagOnly);

    // Regex.lastIndex behavior
    suite.addTest('lastIndex after replace() with "g" flag', testLastIndexGlobalReplace);
    suite.addTest('lastIndex after test() with "g" flag', testLastIndexGlobalTest);

    return suite.run();
}


class CharNumGroups {
    charname: string = 'name';
    num: string = '0';
}

function createHaystack(): string {
    let haystack = "abCdefgz";
    for (let i = 0; i < 3; i++) {
        haystack += haystack;
    }
    return haystack;
}

function verifyRegExp(text: string): string[] {
    text = text.replace(new RegExp("[\\s]", "g"), ' ');
    const emRegExp = new RegExp("<em>.*?</em>", "ig");
    const results = text.split(emRegExp);
    text.match(emRegExp);
    return results;
}

function assertAllMatches(regexp: RegExp, values: string[]): void {
    for (let value of values) {
        arktest.assertTrue(regexp.test(value));
    }
}

function assertBracketExec(pattern: string, input: string, expected: string): void {
    const reg = new RegExp(pattern);
    arktest.assertEQ(reg.exec(input)![0], expected);
}

function testAlternationExecPatterns(): void {
    let reg = new RegExp("ab|cd||");
    arktest.assertEQ(reg.exec("cd")![0], 'cd');

    reg = new RegExp("ab||");
    arktest.assertEQ(reg.exec("cd")![0], '');

    reg = new RegExp("ab|cd|ef");
    arktest.assertEQ(reg.exec("cd")![0], 'cd');
}

function testEmojiReplacementModes(): void {
    const input = "😀";

    let regexp = new RegExp("[😀]");
    arktest.assertEQ(input.replace(regexp, "b"), "b\ude00");

    regexp = new RegExp("[😀]", "g");
    arktest.assertEQ(input.replace(regexp, "b"), "bb");

    regexp = new RegExp("[😀]", "u");
    arktest.assertEQ(input.replace(regexp, "b"), "b");

    regexp = new RegExp("[😀]", "s");
    arktest.assertEQ(input.replace(regexp, "b"), "b\ude00");
}

function testStringReplacementVariants(): void {
    const haystack = createHaystack();
    const replaceFirst = new RegExp("[Cz]");
    const replaceAll = new RegExp("[Cz]", "g");
    const captureFirst = new RegExp("([Cz])");
    const captureAll = new RegExp("([Cz])", "g");
    const emptyGlobal = new RegExp("(?:)", "g");
    const emptyUnicodeGlobal = new RegExp("(?:)", "gu");

    arktest.assertEQ(haystack.replace(replaceFirst, "."), "ab.defgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgz");
    arktest.assertEQ(haystack.replace(replaceAll, "."), "ab.defg.ab.defg.ab.defg.ab.defg.ab.defg.ab.defg.ab.defg.ab.defg.");
    arktest.assertEQ(haystack.replace(replaceAll, "xyz"), "abxyzdefgxyzabxyzdefgxyzabxyzdefgxyzabxyzdefgxyzabxyzdefgxyzabxyzdefgxyzabxyzdefgxyzabxyzdefgxyz");
    arktest.assertEQ(haystack.replace(replaceFirst, "xyz"), "abxyzdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgz");
    arktest.assertEQ("ab".replace(emptyGlobal, "."), ".a.b.");
    arktest.assertEQ("😀".replace(emptyUnicodeGlobal, "."), ".😀.");
    arktest.assertEQ(haystack.replace(captureFirst, "x$1yz"), "abxCyzdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgz");
    arktest.assertEQ(haystack.replace(captureAll, "x$1yz"), "abxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyz");
    arktest.assertEQ(haystack.replace(captureFirst, (match: string) => match), haystack);
    arktest.assertEQ(haystack.replace(captureAll, (match: string) => match), haystack);
}

function testLiteralReplaceFastPath(): void {
    const globalReplace = new RegExp("a", "g");
    arktest.assertEQ("aaa".replace(globalReplace, "b"), "bbb");
    arktest.assertEQ(globalReplace.lastIndex, 0);

    arktest.assertEQ("abc".replace(new RegExp("b"), "X"), "aXc");
    arktest.assertEQ("abc".replace(new RegExp("d", "g"), "X"), "abc");
    arktest.assertEQ("abc".replace(new RegExp("(?:)", "g"), "X"), "XaXbXcX");
    arktest.assertEQ("a😀b".replace(new RegExp("😀", "gu"), "X"), "aXb");
    arktest.assertEQ("a$b".replace(new RegExp("a"), "X"), "X$b");
}

function testReplaceAllGroupCallbacks(): void {
    const namedInput = 'A1B2';
    const namedRegex = new RegExp('(?<charname>[A-Z])(?<num>\\d)', 'g');
    const namedResult = namedInput.replaceAll(
        namedRegex,
        (
            match: string,
            args: Object[],
        ): string => {
            const groups = args[4] as Record<string, string>;
            const myGroups: CharNumGroups = {
                charname: groups.get('charname')!,
                num: groups.get('num')!,
            };
            return `${myGroups.num}${myGroups.charname}`;
        },
    );
    arktest.assertEQ(namedResult, '1A2B');

    const indexedInput = 'A1';
    const indexedRegex = new RegExp('([A-Z])(\\d)', 'g');
    const indexedResult = indexedInput.replaceAll(
        indexedRegex,
        (
            _: string,
            args: Object[],
        ): String => {
            const charGroup = args[0] as String;
            const numGroup = args[1] as String;
            return `${numGroup}${charGroup}`;
        },
    );
    arktest.assertEQ(indexedResult, '1A');
}

function testBasicLiteralAndUrlPatterns(): void {
    let reg = new RegExp("[\\x5d-\\x7e]", "i");
    arktest.assertTrue(reg.test("a"));

    reg = new RegExp("^[-+]?([0-9]+)?(\\٫[0-9]{1,})?$");
    arktest.assertTrue(reg.test('0٫0000000000001'));

    reg = new RegExp("^[A-Z0-9_\\-]*$", "i");
    arktest.assertTrue(reg.test(''));

    reg = new RegExp("^(?<urlStrIndex>.*?)(?<urlStr>https?[:：]//[^/]+/svn(?:/[a-z0-9.,;?'*:+&%$#=~_ \\u4E00-\\u9FA5-]*)*).*$", "i");
    arktest.assertFalse(reg.test('a'));

    reg = new RegExp(
        "^(?<urlStrIndex>.*?)(?<urlStr>(?:(?:ht|f)tps?[:：]//)?(?:[a-z0-9-]+\\.)+"
        + "(?:com|edu|gov|mil|net|org|biz|info|name|museum|us|ca|uk|cn|cc|tw|de|au|sg|hk|ei|fr|me|im)(?![a-z])"
        + "(?:\\:[0-9][0-9]*)?(?:\\.?/[a-z0-9.,;?'\\|*:\\\\+&%$#=~_-]*)*).*$",
        "i",
    );
    arktest.assertFalse(reg.test('a'));

    reg = new RegExp(
        "^(?<urlStrIndex>.*?)(?<urlStr>(?:ht|f)tps?[:：]//(?:[a-z0-9-]+\\.)*[a-z0-9-]+(?:/[a-z0-9]+)*[/a-z0-9.,;?'\\|*:\\\\+&%$#=~_-]*).*$",
        "i",
    );
    arktest.assertFalse(reg.test('a'));

    reg = new RegExp(
        "^(?<urlStrIndex>.*?)(?<urlStr>(?:https?[:：]//)?(?:[a-z0-9-\\\\]+\\.)+"
        + "(?:com|edu|gov|mil|net|org|biz|info|name|museum|us|ca|uk|cn|cc|tw|de|au|sg|hk|ei|fr|me|im)(?![a-z])"
        + "(?:\\:\\d{4})?(?:/[a-z0-9.,;?'\\|*:\\\\+&%$#=~_-]*)*\\?(?:[a-z0-9]*=[a-z0-9.,;?'*:+%$#=~_\\u4E00-\\u9FA5-]*&?)*).*$",
        "i",
    );
    arktest.assertFalse(reg.test('a'));
}

function testUnicodeAlphabetRanges(): void {
    assertAllMatches(
        new RegExp("^([\\s\\x01-\\x08\\x0b\\x0c\\x0e-\\x1f\\x7f\\x21\\x23-\\x5b\\x5d-\\x7e\\u00A0-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFEF]|(\\\\[\\x01-\\x09\\x0b\\x0c\\x0d-\\x7f\\u00A0-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFEF]))*$", "i"),
        [" foo m端ller "],
    );

    assertAllMatches(new RegExp("^[Α-ώ]+$", "iu"), [
        'άέήίΰϊϋόύώ',
        'ΆΈΉΊΪΫΎΏ',
        'αβγδεζηθικλμνξοπρςστυφχψω',
        'ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ',
        'άέήίΰϊϋόύώ',
        'ΆΈΉΊΪΫΎΏ',
    ]);

    assertAllMatches(new RegExp("^[A-ZÃÁÀÂÄÇÉÊËÍÏÕÓÔÖÚÜ]+$", "iu"), [
        'palíndromo',
        'órgão',
        'qwértyúão',
        'àäãcëüïÄÏÜ',
    ]);

    assertAllMatches(new RegExp("^[A-ZÀÉÈÌÎÓÒÙ]+$", "iu"), [
        'àéèìîóòù',
        'metró',
        'pèsca',
        'genî',
    ]);

    assertAllMatches(new RegExp("^[A-ZÀÁẠẢÃÂẦẤẬẨẪĂẰẮẶẲẴĐÈÉẸẺẼÊỀẾỆỂỄÌÍỊỈĨÒÓỌỎÕÔỒỐỘỔỖƠỜỚỢỞỠÙÚỤỦŨƯỪỨỰỬỮỲÝỴỶỸ]+$", "iu"), [
        'thiến',
        'nghiêng',
        'chào',
        'thế',
        'giới',
    ]);

    assertAllMatches(new RegExp("^[A-ZÅÄÖ]+$", "iu"), ['äiti', 'religiös', 'stjäla', 'västgöte']);
    assertAllMatches(new RegExp("^[A-ZÆØÅ]+$", "iu"), ['aøå', 'aøå']);
    assertAllMatches(new RegExp("^[A-ZĄĆĘŚŁŃÓŻŹ]+$", "iu"), [
        'kreską',
        'zamknięte',
        'zwykłe',
        'kropką',
        'przyjęły',
        'święty',
        'Pozwól',
    ]);
    assertAllMatches(new RegExp("^[А-ЯЂЈЉЊЋЏ]+$", "iu"), ['ШћжЂљЕ']);
    assertAllMatches(new RegExp("^[A-ZČĆŽŠĐ]+$", "iu"), ['ŠAabčšđćž', 'ŠATROĆčđš']);
    assertAllMatches(new RegExp("^[A-ZÁÉÍÑÓÚÜ]+$", "iu"), ['ábcó', 'dormís', 'volvés', 'español']);
    assertAllMatches(new RegExp("^[A-ZÇĞİıÖŞÜ]+$", "iu"), ['AİıÖöÇçŞşĞğÜüZ']);
    assertAllMatches(new RegExp("^[A-VXYZÇƏĞİıÖŞÜ]+$", "iu"), [
        'Azərbaycan',
        'üöğıəçş',
        'sizAzərbaycanlaşdırılmışlardansınızmı',
        'dahaBirDüzgünString',
        'abcçdeəfgğhxıijkqlmnoöprsştuüvyz',
    ]);
    assertAllMatches(new RegExp("^[А-Я]+$", "iu"), ['абв', 'жаба', 'яГоДа']);
    assertAllMatches(new RegExp("^[A-ZÁČĎÉĚÍŇÓŘŠŤÚŮÝŽ]+$", "iu"), ['žluťoučký', 'Pěl', 'Ďábelské', 'ódy']);
    assertAllMatches(new RegExp("^[A-ZÁČĎÉÍŇÓŠŤÚÝŽĹŔĽÄÔ]+$", "iu"), ['môj', 'ľúbím', 'mäkčeň', 'vŕba', 'ňorimberk']);
    assertAllMatches(new RegExp("^[A-ZÁÉËÏÓÖÜÚ]+$", "iu"), ['Kán', 'één', 'vóór', 'nú', 'héél']);
    assertAllMatches(new RegExp("^[A-ZÄÖÜß]+$", "iu"), ['äbc', 'FöÖbär']);
    assertAllMatches(new RegExp("^[A-ZÁÉÍÓÖŐÚÜŰ]+$", "iu"), ['árvíztűrőtükörfúrógép']);
}

function testUnicodeAlphaNumericRanges(): void {
    assertAllMatches(new RegExp("^[0-9A-VXYZÇƏĞİıÖŞÜ]+$", "iu"), ['Azərbaycan', 'abcç2', '3kərə4kərə']);
    assertAllMatches(new RegExp("^[0-9А-Я]+$", "iu"), ['абв1', 'жаба', 'яГоДа2', 'йЮя']);
    assertAllMatches(new RegExp("^[0-9A-ZÁČĎÉĚÍŇÓŘŠŤÚŮÝŽ]+$", "iu"), ['řiť123']);
    assertAllMatches(new RegExp("^[0-9A-ZÁČĎÉÍŇÓŠŤÚÝŽĹŔĽÄÔ]+$", "iu"), [
        '1môj',
        '2ľúbím',
        '3mäkčeň',
        '5vŕba',
        '6ňorimberk',
        '7ťava',
        '8žanéta',
        '9Ďábelské',
        '10ódy',
    ]);
    assertAllMatches(new RegExp("^[0-9A-ZÁÉËÏÓÖÜÚ]+$", "iu"), ['Kán123', 'één354']);
    assertAllMatches(new RegExp("^[0-9A-ZÅÄÖ]+$", "iu"), ['äiti124', '451åå23']);
    assertAllMatches(new RegExp("^[0-9A-ZÄÖÜß]+$", "iu"), ['äbc123']);
    assertAllMatches(new RegExp("^[0-9A-ZÁÉÍÓÖŐÚÜŰ]+$", "iu"), ['0árvíztűrőtükörfúrógép123']);
    assertAllMatches(new RegExp("^[0-9A-ZÃÁÀÂÄÇÉÊËÍÏÕÓÔÖÚÜ]+$", "iu"), ['palíndromo', '2órgão', 'qwértyúão9', 'àäãcë4üïÄÏÜ']);
    assertAllMatches(new RegExp("^[0-9A-ZÀÉÈÌÎÓÒÙ]+$", "iu"), ['123àéèìîóòù', 'met23ró', 'pès56ca', 'gen45î']);
    assertAllMatches(new RegExp("^[0-9A-ZÁÉÍÑÓÚÜ]+$", "iu"), ['ábcó123']);
    assertAllMatches(new RegExp("^[0-9A-ZÀÁẠẢÃÂẦẤẬẨẪĂẰẮẶẲẴĐÈÉẸẺẼÊỀẾỆỂỄÌÍỊỈĨÒÓỌỎÕÔỒỐỘỔỖƠỜỚỢỞỠÙÚỤỦŨƯỪỨỰỬỮỲÝỴỶỸ]+$", "iu"), ['Thầy3', '3Gà']);
    assertAllMatches(new RegExp("^[0-9A-ZĄĆĘŚŁŃÓŻŹ]+$", "iu"), ['kre123ską', 'zam21knięte', 'zw23ykłe', 'prz23yjęły', 'świ23ęty', 'Poz1322wól']);
    assertAllMatches(new RegExp("^[0-9А-ЯЂЈЉЊЋЏ]+$", "iu"), ['ШћжЂљЕ123']);
    assertAllMatches(new RegExp("^[0-9A-ZČĆŽŠĐ]+$", "iu"), ['ŠAabčšđćž123', 'ŠATRO11Ćčđš']);
    assertAllMatches(new RegExp("^[0-9A-ZÅÄÖ]+$", "iu"), ['religiös13', 'st23jäla', 'västgöte123']);
    assertAllMatches(new RegExp("^[0-9A-ZÇĞİıÖŞÜ]+$", "iu"), ['AİıÖöÇçŞşĞğÜüZ123']);
}

function testNumericAndCurrencyPatterns(): void {
    assertAllMatches(new RegExp("^[-+]?([0-9]+)?(\\٫[0-9]{1,})?$"), ['0٫0000000000001']);
    assertAllMatches(new RegExp("^(?:[-+])?(?:[0-9]+)?(?:\\٫[0-9]*)?(?:[eE][\\+\\-]?(?:[0-9]+))?$"), ['123٫', '123٫123', '-123٫123']);
    assertAllMatches(new RegExp("^[A-Z0-9_\\-]*$", "iu"), ['']);
    assertAllMatches(new RegExp("^(?!-? )(?=.*\\d)(\\¥)?-?(0|[1-9]\\d|[1-9]\\d{0,2}(\\,\\d{3})*)?(\\.(\\d{2}))?$"), [
        '¥6,954,231',
        '¥-6,954,231',
    ]);
}

function testBase64AlphabetPattern(): void {
    assertAllMatches(new RegExp("^[A-Z0-9+/=]*$", "i"), [
        "Zg==",
        "Zm8=",
        "Zm9v",
        "Zm9vYg==",
        "Zm9vYmE=",
        "Zm9vYmFy",
        "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdC4=",
        "Vml2YW11cyBmZXJtZW50dW0gc2VtcGVyIHBvcnRhLg==",
        "U3VzcGVuZGlzc2UgbGVjdHVzIGxlbw==",
    ]);
}

function testCacheSensitiveReplaceAndTestLoops(): void {
    const sanitized = 'SC52BAHL01031234567890123456USD';
    arktest.assertEQ(sanitized.replace(new RegExp("[^A-Z0-9]+", "gi"), ''), "SC52BAHL01031234567890123456USD");

    const regabc = new RegExp("abc", "g");
    const strabcd = "abcdabcdabcd";
    const expected: boolean[] = [true, true, true, false, true, true, true, false, true, true];
    const actual: boolean[] = [];
    for (let i = 0; i < 10; i++) {
        actual.push(regabc.test(strabcd));
    }
    arktest.assertEQ(expected.toString(), actual.toString());

    const str = "aaaabbBbcccC";
    for (let i = 0; i < 2; i++) {
        arktest.assertEQ(str, 'aaaabbBbcccC');
        const callbackReplaced = str.replace(new RegExp("([A-Z])", "g"), (e: string) => {
            return "_" + e;
        });
        arktest.assertEQ(callbackReplaced, 'aaaabb_Bbccc_C');

        const captureReplaced = str.replace(new RegExp("([A-Z])", "g"), "_$1");
        arktest.assertEQ(captureReplaced, 'aaaabb_Bbccc_C');
        arktest.assertEQ(callbackReplaced.replace(new RegExp("([a-z]+)", "g"), "_xy"), '_xy_B_xy_C');
        arktest.assertEQ(captureReplaced.replace(new RegExp("_", "g"), ""), 'aaaabbBbcccC');
    }
}

function testLastIndexAwareExecBehavior(): void {
    const regexp1 = new RegExp("a*", "gs");
    const regexp2 = new RegExp("a*", "g");
    regexp2.lastIndex = 8;
    arktest.assertEQ(regexp1.exec('aaa')?.toString(), 'aaa');
    arktest.assertEQ(regexp2.exec('aaa'), null);

    const regexp3 = new RegExp("a*", "g");
    const regexp4 = new RegExp("a*", "g");
    regexp4.lastIndex = 1;
    arktest.assertEQ(regexp3.exec('aaabab')?.toString(), 'aaa');
    arktest.assertEQ(regexp4.exec('aaabaa')?.toString(), 'aa');
}

function testReplaceCachingPreservesLastIndex(): void {
    const replaceFirst = new RegExp("[Cz]");
    const replaceAll = new RegExp("[Cz]", "g");
    const captureFirst = new RegExp("([Cz])");
    const captureAll = new RegExp("([Cz])", "g");
    const input = "abCdefgzabCdefgzabCdefgz";

    let replaced = input.replace(replaceFirst, "");
    arktest.assertEQ(replaceFirst.lastIndex, 0);
    let cached = input.replace(replaceFirst, "");
    arktest.assertTrue(replaced === cached);
    arktest.assertEQ(replaceFirst.lastIndex, 0);

    replaced = input.replace(replaceAll, "xyz");
    arktest.assertEQ(replaceAll.lastIndex, 0);
    cached = input.replace(replaceAll, "xyz");
    arktest.assertTrue(replaced === cached);
    arktest.assertEQ(replaceAll.lastIndex, 0);

    replaced = input.replace(captureFirst, "x$1yz");
    arktest.assertEQ(captureFirst.lastIndex, 0);
    cached = input.replace(captureFirst, "x$1yz");
    arktest.assertTrue(replaced === cached);
    arktest.assertEQ(captureFirst.lastIndex, 0);

    replaced = input.replace(captureAll, (match: string) => match);
    arktest.assertEQ(captureAll.lastIndex, 0);
    cached = input.replace(captureAll, (match: string) => match);
    arktest.assertTrue(replaced === cached);
    arktest.assertEQ(captureAll.lastIndex, 0);
}

function testSplitAndMarkupExtractionPatterns(): void {
    const inputString = "/vedio/av{avid}{cid}";
    const extractedContent = inputString.match(new RegExp("\\{([^{}]+)\\}", "g"));
    arktest.assertEQ(extractedContent!.toString(), '{avid},{cid}');

    const replacedString = inputString
        .replace(new RegExp("\\{([^{}]+)\\}", "g"), '(uuu)')
        .replace(new RegExp("\\/", "g"), "\\/");
    arktest.assertEQ(replacedString, '\\/vedio\\/av(uuu)(uuu)');

    const splitInput = "beep boop   afff测试样本one1";
    arktest.assertEQ(splitInput.split(new RegExp("([{}:;,]|\\s+)")).toString(), 'beep, ,boop,   ,afff测试样本one1');

    const srcTextA = '<em>a</em>bcdefghijklmnopqrstuvwxyz<em>a</em>bcdefghijklmnopqrstuvwxyz<em>a</em>bcdefghijklmnopqrstuvwxyz<em>a</em>bcdefghijklmnopqrstuvwxyz';
    const srcTextAbcd = '<em>abcd</em>efghijklmnopqrstuvwxyz<em>abcd</em>efghijklmnopqrstuvwxyz<em>abcd</em>efghijklmnopqrstuvwxyz<em>abcd</em>efghijklmnopqrstuvwxyz';

    const s0 = `verifyRegExp_1: ${verifyRegExp(srcTextA)}`;
    arktest.assertEQ(s0, 'verifyRegExp_1: ,bcdefghijklmnopqrstuvwxyz,bcdefghijklmnopqrstuvwxyz,bcdefghijklmnopqrstuvwxyz,bcdefghijklmnopqrstuvwxyz');
    const s1 = `verifyRegExp_2: ${verifyRegExp(srcTextAbcd)}`;
    arktest.assertEQ(s1, 'verifyRegExp_2: ,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz');
    const s2 = `verifyRegExp_3: ${verifyRegExp(srcTextAbcd)}`;
    arktest.assertEQ(s2, 'verifyRegExp_3: ,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz');
}

function testSplitSearchAndMatchStateBehavior(): void {
    arktest.expectThrow((): void => { new RegExp("/[\\p{}]/u"); }, (e) => e instanceof SyntaxError);

    const splitInput = "a-b-c";
    const sticky = new RegExp("-", "y");
    arktest.assertEQ(splitInput.split(sticky).toString(), 'a,b,c');

    sticky.lastIndex = 1;
    arktest.assertEQ(splitInput.split(sticky).toString(), 'a,b,c');
    arktest.assertEQ(sticky.split(splitInput, undefined).toString(), 'a,b,c');

    sticky.lastIndex = -1;
    arktest.assertEQ(splitInput.split(sticky).toString(), 'a,b,c');

    sticky.lastIndex = 3;
    arktest.assertEQ(splitInput.split(sticky).toString(), 'a,b,c');
    arktest.assertTrue(sticky.test(splitInput));
    arktest.assertEQ(splitInput.split(new RegExp("-", "g")).toString(), 'a,b,c');

    const searchInput = "abc";
    const searchRegExp = new RegExp("b");
    searchRegExp.lastIndex = 2;
    arktest.assertEQ(searchInput.search(searchRegExp), 1);
    arktest.assertEQ(searchInput.search(new RegExp("b", "y")), -1);
    arktest.assertEQ(searchInput.search(searchRegExp), 1);
    arktest.assertEQ(searchRegExp.lastIndex, 2);

    const cachedSplitInput = "a-bc";
    const splitRegExp = new RegExp("-");
    splitRegExp.lastIndex = 2;
    arktest.assertEQ(cachedSplitInput.split(splitRegExp).toString(), 'a,bc');
    arktest.assertEQ(splitRegExp.lastIndex, 2);
    arktest.assertEQ(cachedSplitInput.split(splitRegExp).toString(), 'a,bc');
    arktest.assertEQ(splitRegExp.lastIndex, 2);

    const matchInput = "abcabc";
    const singleMatchRegExp = new RegExp("abc");
    singleMatchRegExp.lastIndex = 2;
    arktest.assertEQ(matchInput.match(singleMatchRegExp)!.toString(), 'abc');
    arktest.assertEQ(singleMatchRegExp.lastIndex, 2);
    arktest.assertEQ(matchInput.match(singleMatchRegExp)!.toString(), 'abc');
    arktest.assertEQ(singleMatchRegExp.lastIndex, 2);

    const globalMatchRegExp = new RegExp("abc", "g");
    globalMatchRegExp.lastIndex = 2;
    arktest.assertEQ(matchInput.match(globalMatchRegExp)!.toString(), 'abc,abc');
    arktest.assertEQ(globalMatchRegExp.lastIndex, 0);
    arktest.assertEQ(matchInput.match(globalMatchRegExp)!.toString(), 'abc,abc');
    arktest.assertEQ(globalMatchRegExp.lastIndex, 0);
}

function testEmptyAlternativeAndExecBehavior(): void {
    const regex = new RegExp("(?:)+");
    const input = "abcabcabc";
    arktest.assertTrue(regex.test(input));
    arktest.assertEQ(regex.exec(input)!.toString(), "");
}

function testRegexFlagReporting(): void {
    const withAllFlags = new RegExp("abc", "giymsud");
    withAllFlags.lastIndex = -1;
    arktest.assertEQ(withAllFlags.flags, 'dgimsuy');

    const global = new RegExp("abc", "g");
    global.lastIndex = -1;
    arktest.assertEQ(global.flags, 'g');

    const ignoreCase = new RegExp("abc", "i");
    ignoreCase.lastIndex = -1;
    arktest.assertEQ(ignoreCase.flags, 'i');

    const sticky = new RegExp("abc", "y");
    sticky.lastIndex = -1;
    arktest.assertEQ(sticky.flags, 'y');

    const multiline = new RegExp("abc", "m");
    multiline.lastIndex = -1;
    arktest.assertEQ(multiline.flags, 'm');

    const dotAll = new RegExp("abc", "s");
    dotAll.lastIndex = -1;
    arktest.assertEQ(dotAll.flags, 's');

    const unicode = new RegExp("abc", "u");
    unicode.lastIndex = -1;
    arktest.assertEQ(unicode.flags, 'u');

    const indices = new RegExp("abc", "d");
    indices.lastIndex = -1;
    arktest.assertEQ(indices.flags, 'd');

    const plain = new RegExp("abc");
    plain.lastIndex = -1;
    arktest.assertEQ(plain.flags, '');
}

function testEscapedBracketPatterns(): void {
    const input = "\\\\\\\\[aaa";

    arktest.expectThrow((): void => { new RegExp("["); }, (e) => e instanceof SyntaxError);
    arktest.expectThrow((): void => { new RegExp("\["); }, (e) => e instanceof SyntaxError);
    assertBracketExec("\\[", input, '[');
    assertBracketExec("\\\[", input, '[');
    arktest.expectThrow((): void => { new RegExp("\\\\["); }, (e) => e instanceof SyntaxError);
    arktest.expectThrow((): void => { new RegExp("\\\\\["); }, (e) => e instanceof SyntaxError);
    assertBracketExec("\\\\\\[", input, '\\[');
    assertBracketExec("\\\\\\\[", input, '\\[');
}

function testUtf8ReplacementBranches(): void {
    let input = 'utf83c这要替换"!';
    let regexp = new RegExp("([0-9])([a-z])", "g");
    let replaced = input.replace(regexp, "$1");
    arktest.assertEQ(replaced, 'utf83这要替换"!');

    input = '3c这要替换"!';
    regexp = new RegExp("([0-9])([a-z])", "g");
    replaced = input.replace(regexp, "$1");
    arktest.assertEQ(replaced, '3这要替换"!');

    input = 'abcdefg3chigk"!';
    regexp = new RegExp("([0-9])([a-z])", "g");
    replaced = input.replace(regexp, "$1");
    arktest.assertEQ(replaced, 'abcdefg3higk"!');
}

function testLargePatternReplacement(): void {
    const asciiToken = 'abc';
    const unicodeToken = '\ud834\udf06';

    let reg = new RegExp(asciiToken.repeat(10000));
    let input = asciiToken.repeat(10000);
    let replaced = input.replace(reg, unicodeToken);
    arktest.assertTrue(replaced == unicodeToken);

    reg = new RegExp(unicodeToken.repeat(10000));
    input = unicodeToken.repeat(10000);
    replaced = input.replace(reg, asciiToken);
    arktest.assertTrue(replaced == asciiToken);
}

function testOptionalGroupExec(): void {
    const reg = new RegExp("a(?:|x)$");
    arktest.assertEQ(reg.exec("ax")![0], 'ax');
}

function testRegexDotAllUnicodeFlags(): void {
    const rep1 = new RegExp('^.$', 'su');
    const rep2 = new RegExp('^.$', 'sum');
    const arr: Array<RegExp> = new Array<RegExp>(rep1, rep2);

    for (let re of arr) {
        arktest.assertTrue(re.test('a'));
        arktest.assertTrue(re.test('3'));
        arktest.assertTrue(re.test('π'));
        arktest.assertTrue(re.test('\u2027'));
        arktest.assertTrue(re.test('\u0085'));
        arktest.assertTrue(re.test('\v'));
        arktest.assertTrue(re.test('\f'));
        arktest.assertTrue(re.test('\u180E'));
        arktest.assertTrue(re.test('\u{10300}'));
        arktest.assertTrue(re.test('\n'));
        arktest.assertTrue(re.test('\r'));
        arktest.assertTrue(re.test('\u2028'));
        arktest.assertTrue(re.test('\u2029'));
        arktest.assertTrue(re.test('\uD800'));
        arktest.assertTrue(re.test('\uDFFF'));
    }
}

function testRegexUnicodeFlagOnly(): void {
    const rep1 = new RegExp('^.$', 'u');
    const rep2 = new RegExp('^.$', 'um');
    const arr: Array<RegExp> = new Array<RegExp>(rep1, rep2);

    for (let re of arr) {
        arktest.assertTrue(re.test('a'));
        arktest.assertTrue(re.test('3'));
        arktest.assertTrue(re.test('π'));
        arktest.assertTrue(re.test('\u2027'));
        arktest.assertTrue(re.test('\u0085'));
        arktest.assertTrue(re.test('\v'));
        arktest.assertTrue(re.test('\f'));
        arktest.assertTrue(re.test('\u180E'));
        arktest.assertTrue(re.test('\u{10300}'));
        arktest.assertFalse(re.test('\n'));
        arktest.assertFalse(re.test('\r'));
        arktest.assertFalse(re.test('\u2028'));
        arktest.assertFalse(re.test('\u2029'));
        arktest.assertTrue(re.test('\uD800'));
        arktest.assertTrue(re.test('\uDFFF'));
    }
}

function testLastIndexGlobalReplace() {
    const str = "abc abc abc";
    const regex = new RegExp('abc', 'g');
    regex.lastIndex = 4;
    const result = str.replace(regex, "XYZ");
    arktest.assertEQ(result, "XYZ XYZ XYZ");
    arktest.assertEQ(regex.lastIndex, 0);
}

function testLastIndexGlobalTest() {
    let re = new RegExp("a", "g")
    re.lastIndex = 2
    let r1 = re.test("a")
    arktest.assertFalse(r1)
    arktest.assertEQ(re.lastIndex, 0)
    let r2 = re.test("a")
    arktest.assertTrue(r2)
    arktest.assertEQ(re.lastIndex, 1)
}