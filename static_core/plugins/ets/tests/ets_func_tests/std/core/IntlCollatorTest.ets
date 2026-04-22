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

function main(): int {
    const suite = new arktest.ArkTestsuite('Intl.Collator');

    suite.addTest('Intl.Collator.compare', testCollatorCompare);
    suite.addTest('Intl.Collator constructor locales undefined uses default locale', testCollatorUndefinedLocales);
    suite.addTest('Array.sort call Intl.Collator.compare', testArraySortCompare);
    return suite.run();
}

function testCollatorCompare(): void {
    // 1. Basic English sorting
    let collator = new Intl.Collator('en');
    arktest.assertLT(collator.compare('a', 'b'), 0, "Basic sort: 'a' should be less than 'b'")
    arktest.assertTrue(collator.compare('b', 'a') > 0, "Basic sort: 'b' should be greater than 'a'");
    arktest.assertEQ(collator.compare('a', 'a'), 0, "Basic sort: 'a' should be equal to 'a'");

    // 2. German sorting (diacritics are sorted after base letter)
    collator = new Intl.Collator('de');
    arktest.assertLT(collator.compare('ä', 'z'), 0, "German sort: 'ä' should be less than 'z'");
    arktest.assertLT(collator.compare('a', 'ä'), 0, "German sort: 'a' should be less than 'ä'");

    // 3. Swedish sorting (some letters are at the end of the alphabet)
    collator = new Intl.Collator('sv');
    arktest.assertLT(collator.compare('z', 'ö'), 0, "Swedish sort: 'z' should be less than 'ö'");

    //4. Numeric sorting
    let numericCollator = new Intl.Collator('en', { numeric: true });
    arktest.assertLT(numericCollator.compare('2', '10'), 0, "Numeric sort: '2' should be less than '10'");
    let nonNumericCollator = new Intl.Collator('en', { numeric: false });
    arktest.assertTrue(nonNumericCollator.compare('2', '10') > 0, "Non-numeric sort: '2' should be greater than '10'");

    // 5. Sensitivity option
    let baseCollator = new Intl.Collator('en', { sensitivity: 'base' });
    arktest.assertEQ(baseCollator.compare('a', 'A'), 0, "Sensitivity 'base': 'a' should equal 'A'");
    arktest.assertEQ(baseCollator.compare('a', 'á'), 0, "Sensitivity 'base': 'a' should equal 'á'");

    let accentCollator = new Intl.Collator('en', { sensitivity: 'accent' });
    arktest.assertEQ(accentCollator.compare('a', 'A'), 0, "Sensitivity 'accent': 'a' should equal 'A'");
    arktest.assertLT(accentCollator.compare('a', 'á'), 0, "Sensitivity 'accent': 'a' should be less than 'á'");
    
    // 6. caseFirst option
    let upperFirstCollator = new Intl.Collator('sv', { caseFirst: 'upper' });
    arktest.assertTrue(upperFirstCollator.compare('a', 'A') > 0, "caseFirst 'upper': 'a' should be greater than 'A'");
    
    let lowerFirstCollator = new Intl.Collator('sv', { caseFirst: 'lower' });
    arktest.assertLT(lowerFirstCollator.compare('a', 'A'), 0, "caseFirst 'lower': 'a' should be less than 'A'");
    
    // 7. ignorePunctuation option
    let punctuationCollator = new Intl.Collator('en', { ignorePunctuation: true });
    arktest.assertEQ(punctuationCollator.compare('a-b', 'ab'), 0, "ignorePunctuation: 'a-b' should equal 'ab'");
}

function testArraySortCompare(): void {
    let arr = ['a', 'c', 'b'];
    let col = new Intl.Collator('de', {sensitivity: 'base'});
    arr.sort(col.compare);
    arktest.assertEQ(arr.join(), 'a,b,c');
}

/**
 * When `locales` is omitted or `undefined`, the constructor skips locale resolution and keeps
 * `locale: Locale.defaultTag()` (see Intl.Collator constructor).
 */
function testCollatorUndefinedLocales(): void {
    const defaultTag = Intl.Locale.defaultTag();

    const omitted = new Intl.Collator();
    arktest.assertEQ(omitted.resolvedOptions().locale, defaultTag, 'no-arg: resolved locale should be default');

    const explicitUndefined = new Intl.Collator(undefined);
    arktest.assertEQ(explicitUndefined.resolvedOptions().locale, defaultTag, 'undefined locales: resolved locale');

    const explicitUndefinedTwice = new Intl.Collator(undefined, undefined);
    arktest.assertEQ(explicitUndefinedTwice.resolvedOptions().locale, defaultTag, 'undefined locales and options');

    const withOptionsOnly = new Intl.Collator(undefined, { sensitivity: 'base' });
    arktest.assertEQ(withOptionsOnly.resolvedOptions().locale, defaultTag, 'options only: locale stays default');
    arktest.assertEQ(withOptionsOnly.resolvedOptions().sensitivity, 'base', 'options only: sensitivity applied');
    arktest.assertEQ(withOptionsOnly.compare('a', 'A'), 0, 'base sensitivity on default locale');

    arktest.assertLT(omitted.compare('a', 'b'), 0);
    arktest.assertEQ(omitted.compare('x', 'x'), 0);
}
