/*
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
    const suite = new arktest.ArkTestsuite('String.replaceAll');

    suite.addTest('literal replacement without substitutions', testLiteralReplacementWithoutSubstitutions);
    suite.addTest('replacement metacharacters', testReplacementMetacharacters);
    suite.addTest('numeric replacement tokens without captures', testNumericReplacementTokensWithoutCaptures);
    suite.addTest('named replacement tokens without captures', testNamedReplacementTokensWithoutCaptures);
    suite.addTest('empty search string behavior', testEmptySearchStringBehavior);
    suite.addTest('no match returns original content', testNoMatchReturnsOriginalContent);

    return suite.run();
}

function testLiteralReplacementWithoutSubstitutions(): void {
    arktest.assertEQ('foo_XXfoo_XX', 'foo_bar_foo_bar_'.replaceAll('bar_', 'XX'));
    arktest.assertEQ('bbb', 'aaa'.replaceAll('a', 'b'));
}

function testReplacementMetacharacters(): void {
    const input = 'abcabc';
    arktest.assertEQ('a$ca$c', input.replaceAll('b', '$$'));
    arktest.assertEQ('abcabccabcc', input.replaceAll('b', '$&$\''));
    arktest.assertEQ('aacaabcac', input.replaceAll('b', '$`'));
    arktest.assertEQ('acabccacc', input.replaceAll('b', '$\''));
    arktest.assertEQ('a$&ca$&c', input.replaceAll('b', '$$&'));
}

function testNumericReplacementTokensWithoutCaptures(): void {
    arktest.assertEQ('a$1ca$1c', 'abcabc'.replaceAll('b', '$1'));
    arktest.assertEQ('a$01ca$01c', 'abcabc'.replaceAll('b', '$01'));
    arktest.assertEQ('a$12ca$12c', 'abcabc'.replaceAll('b', '$12'));
    arktest.assertEQ('x$0y', 'xay'.replaceAll('a', '$0'));
}

function testNamedReplacementTokensWithoutCaptures(): void {
    arktest.assertEQ('a$<name>ca$<name>c', 'abcabc'.replaceAll('b', '$<name>'));
    arktest.assertEQ('a$<ca$<c', 'abcabc'.replaceAll('b', '$<'));
    arktest.assertEQ('a$<>ca$<>c', 'abcabc'.replaceAll('b', '$<>'));
}

function testEmptySearchStringBehavior(): void {
    arktest.assertEQ('-a-b-c-', 'abc'.replaceAll('', '-'));
    arktest.assertEQ('$a$b$c$', 'abc'.replaceAll('', '$$'));
    arktest.assertEQ('abc', 'abc'.replaceAll('', '$&'));
}

function testNoMatchReturnsOriginalContent(): void {
    const input = 'abc';
    arktest.assertEQ(input, input.replaceAll('z', '$$'));
    arktest.assertEQ(input, input.replaceAll('z', 'replacement'));
}
