/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

function testStringlocaleCompare1Param() {
    let str:string = "5";
    arktest.assertEQ(str.localeCompare('3'), 1);
    arktest.assertEQ(str.localeCompare('7'), -1);
    arktest.assertEQ(str.localeCompare('5'), 0);
}

function testStringlocaleCompare2Param() {
    let str:string = "5";
    arktest.assertEQ(str.localeCompare('3', "zh-CN"), 1);
    arktest.assertEQ(str.localeCompare('3', undefined), 1);
    arktest.assertEQ(str.localeCompare('7', "zh-CN"), -1);
    arktest.assertEQ(str.localeCompare('5', "zh-CN"), 0);
}

/**
 * Default collator path: `locale` and `options` are undefined (omitted or explicit `undefined`).
 * Must match `new Intl.Collator()` for the same strings.
 */
function testLocaleCompareUndefinedLocaleUsesDefaultCollator() {
    const defCollator = new Intl.Collator();

    arktest.assertEQ("5".localeCompare("3"), defCollator.compare("5", "3"));
    arktest.assertEQ("5".localeCompare("3", undefined), defCollator.compare("5", "3"));
    arktest.assertEQ("5".localeCompare("3", undefined, undefined), defCollator.compare("5", "3"));

    arktest.assertEQ("low".localeCompare("high"), defCollator.compare("low", "high"));
    arktest.assertEQ("low".localeCompare("high", undefined), defCollator.compare("low", "high"));

    arktest.assertEQ("α".localeCompare("β"), defCollator.compare("α", "β"));
}

function testStringlocaleCompare3Param() {
    let su: Intl.CollatorOptions = {
        usage: 'sort',
        localeMatcher: "lookup"
    }

    let str:string = "5";
    arktest.assertEQ(str.localeCompare('3', "zh-CN", su), 1);
    arktest.assertEQ(str.localeCompare('7', "zh-CN", su), -1);
    arktest.assertEQ(str.localeCompare('5', "zh-CN", su), 0);
    arktest.assertEQ(str.localeCompare('3', ["zh-CN", "en-US"], su), 1);
    arktest.assertEQ(str.localeCompare('7', ["zh-CN", "en-US"], su), -1);
    arktest.assertEQ(str.localeCompare('5', ["zh-CN", "en-US"], su), 0);
    arktest.assertEQ(str.localeCompare('3', ["error-locale", "en-US"], su), 1);
    arktest.assertEQ(str.localeCompare('7', ["error-locale", "en-US"], su), -1);
    arktest.assertEQ(str.localeCompare('5', ["error-locale", "en-US"], su), 0);
}

function testDefaultPathCacheConsistency() {
    arktest.assertEQ("a".localeCompare("b"), -1);
    arktest.assertEQ("b".localeCompare("a"), 1);
    arktest.assertEQ("abc".localeCompare("abc"), 0);

    arktest.assertEQ("apple".localeCompare("banana"), -1);
    arktest.assertEQ("banana".localeCompare("apple"), 1);
    arktest.assertEQ("cherry".localeCompare("cherry"), 0);

    arktest.assertEQ("A".localeCompare("a"), 1);
    arktest.assertEQ("a".localeCompare("A"), -1);
}

function testDefaultPathRepeatedCalls() {
    for (let i = 0; i < 100; i++) {
        arktest.assertEQ("x".localeCompare("y"), -1);
        arktest.assertEQ("y".localeCompare("x"), 1);
        arktest.assertEQ("z".localeCompare("z"), 0);
    }
}

function testDefaultPathEdgeCases() {
    arktest.assertEQ("".localeCompare(""), 0);
    arktest.assertEQ("".localeCompare("a"), -1);
    arktest.assertEQ("a".localeCompare(""), 1);

    arktest.assertEQ("a".localeCompare("a"), 0);

    const long1 = "a".repeat(500);
    const long2 = "a".repeat(500);
    arktest.assertEQ(long1.localeCompare(long2), 0);

    const long3 = "a".repeat(499) + "b";
    arktest.assertEQ(long1.localeCompare(long3), -1);
}

function testSensitivityBase() {
    const opts: Intl.CollatorOptions = { sensitivity: "base" }

    arktest.assertEQ("a".localeCompare("A", "en-US", opts), 0);
    arktest.assertEQ("HELLO".localeCompare("hello", "en-US", opts), 0);
    arktest.assertEQ("Hello".localeCompare("hElLo", "en-US", opts), 0);

    arktest.assertEQ("résumé".localeCompare("resume", "en-US", opts), 0);
    arktest.assertEQ("café".localeCompare("cafe", "en-US", opts), 0);
    arktest.assertEQ("naïve".localeCompare("naive", "en-US", opts), 0);

    arktest.assertEQ("a".localeCompare("b", "en-US", opts), -1);
    arktest.assertEQ("z".localeCompare("a", "en-US", opts), 1);
}

function testSensitivityAccent() {
    const opts: Intl.CollatorOptions = { sensitivity: "accent" }

    arktest.assertEQ("a".localeCompare("A", "en-US", opts), 0);
    arktest.assertEQ("HELLO".localeCompare("hello", "en-US", opts), 0);

    arktest.assertTrue("résumé".localeCompare("resume", "en-US", opts) != 0);
    arktest.assertTrue("café".localeCompare("cafe", "en-US", opts) != 0);
}

function testSensitivityCase() {
    const opts: Intl.CollatorOptions = { sensitivity: "case" }

    arktest.assertTrue("a".localeCompare("A", "en-US", opts) != 0);

    arktest.assertEQ("résumé".localeCompare("résumé", "en-US", opts), 0);
}

function testSensitivityVariant() {
    const opts: Intl.CollatorOptions = { sensitivity: "variant" }

    arktest.assertTrue("a".localeCompare("A", "en-US", opts) != 0);
    arktest.assertTrue("résumé".localeCompare("resume", "en-US", opts) != 0);
    arktest.assertEQ("abc".localeCompare("abc", "en-US", opts), 0);
}

function testSensitivityBaseRepeated() {
    const opts: Intl.CollatorOptions = { sensitivity: "base" }
    for (let i = 0; i < 50; i++) {
        arktest.assertEQ("Hello".localeCompare("hello", "en-US", opts), 0);
        arktest.assertEQ("café".localeCompare("cafe", "en-US", opts), 0);
        arktest.assertEQ("a".localeCompare("b", "en-US", opts), -1);
    }
}

function testMixedCachedAndNonCached() {
    arktest.assertEQ("a".localeCompare("b"), -1);

    arktest.assertEQ("a".localeCompare("b", "en-US"), -1);

    arktest.assertEQ("a".localeCompare("b"), -1);
    arktest.assertEQ("x".localeCompare("x"), 0);

    const opts: Intl.CollatorOptions = { sensitivity: "base" }
    arktest.assertEQ("A".localeCompare("a", "en-US", opts), 0);

    arktest.assertEQ("A".localeCompare("a"), 1);
    arktest.assertEQ("b".localeCompare("a"), 1);
}

function testLocaleSpecificOrdering() {
    arktest.assertEQ("ä".localeCompare("z", "de"), -1);
    arktest.assertEQ("ä".localeCompare("z", "sv"), 1);
}

function testCollatorWithoutLocale() {
    const c = new Intl.Collator();
    arktest.assertEQ(c.compare("a", "b"), -1);
    arktest.assertEQ(c.compare("b", "a"), 1);
    arktest.assertEQ(c.compare("abc", "abc"), 0);
    arktest.assertEQ(c.compare("", ""), 0);
    arktest.assertEQ(c.compare("", "a"), -1);

    for (let i = 0; i < 50; i++) {
        arktest.assertEQ(c.compare("x", "y"), -1);
    }
}

function testCollatorWithLocale() {
    const cDe = new Intl.Collator("de");
    arktest.assertEQ(cDe.compare("ä", "z"), -1);

    const cSv = new Intl.Collator("sv");
    arktest.assertEQ(cSv.compare("ä", "z"), 1);
}

function testCollatorSensitivityBase() {
    const c = new Intl.Collator("en-US", { sensitivity: "base" } as Intl.CollatorOptions);
    arktest.assertEQ(c.compare("A", "a"), 0);
    arktest.assertEQ(c.compare("résumé", "resume"), 0);
    arktest.assertEQ(c.compare("café", "cafe"), 0);
    arktest.assertEQ(c.compare("a", "b"), -1);
}

function testUnicodeNormalization() {
    arktest.assertEQ("ö".localeCompare("o\u0308"), 0);
    arktest.assertEQ("Å".localeCompare("A\u030A"), 0);
    arktest.assertEQ("Ç".localeCompare("C\u0327"), 0);
    arktest.assertEQ("ô".localeCompare("o\u0302"), 0);
}

function testSpecialCharacters() {
    arktest.assertEQ("hello world".localeCompare("hello world"), 0);
    arktest.assertEQ("hello!".localeCompare("hello!"), 0);
    arktest.assertEQ("123".localeCompare("123"), 0);

    arktest.assertEQ("日本語".localeCompare("日本語"), 0);
    arktest.assertEQ("中文".localeCompare("中文"), 0);
    arktest.assertEQ("한국어".localeCompare("한국어"), 0);
    arktest.assertEQ("العربية".localeCompare("العربية"), 0);
}

function main(): int {
    const suite = new arktest.ArkTestsuite("String.localeCompare");
    suite.addTest("Test StringlocaleCompare1Param", testStringlocaleCompare1Param);
    suite.addTest("Test StringlocaleCompare2Param", testStringlocaleCompare2Param);
    suite.addTest("localeCompare undefined locale matches default Intl.Collator", testLocaleCompareUndefinedLocaleUsesDefaultCollator);
    suite.addTest("Test StringlocaleCompare3Param", testStringlocaleCompare3Param);
    suite.addTest("default path: cache consistency", testDefaultPathCacheConsistency);
    suite.addTest("default path: repeated calls", testDefaultPathRepeatedCalls);
    suite.addTest("default path: edge cases", testDefaultPathEdgeCases);
    suite.addTest("sensitivity: base", testSensitivityBase);
    suite.addTest("sensitivity: accent", testSensitivityAccent);
    suite.addTest("sensitivity: case", testSensitivityCase);
    suite.addTest("sensitivity: variant", testSensitivityVariant);
    suite.addTest("sensitivity: base repeated", testSensitivityBaseRepeated);
    suite.addTest("mixed cached and non-cached calls", testMixedCachedAndNonCached);
    suite.addTest("locale-specific ordering (de vs sv)", testLocaleSpecificOrdering);
    suite.addTest("Collator without locale", testCollatorWithoutLocale);
    suite.addTest("Collator with locale", testCollatorWithLocale);
    suite.addTest("Collator sensitivity base", testCollatorSensitivityBase);
    suite.addTest("unicode normalization", testUnicodeNormalization);
    suite.addTest("special characters", testSpecialCharacters);
    return suite.run();
}
