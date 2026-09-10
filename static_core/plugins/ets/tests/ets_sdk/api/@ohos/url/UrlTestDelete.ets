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
import { url } from '@ohos.url';

function main(): int {
    let suite = new arktest.ArkTestsuite('URL delete tests');

    suite.addTest('testDeleteHead', testDeleteHead);
    suite.addTest('testDeleteTail', testDeleteTail);
    suite.addTest('testDeleteMiddle', testDeleteMiddle);
    suite.addTest('testDeleteOnlyEntry', testDeleteOnlyEntry);
    suite.addTest('testDeleteNonExistent', testDeleteNonExistent);
    suite.addTest('testDeleteFromEmpty', testDeleteFromEmpty);
    suite.addTest('testDeleteMultipleAtHead', testDeleteMultipleAtHead);
    suite.addTest('testDeleteMultipleAtTail', testDeleteMultipleAtTail);
    suite.addTest('testDeleteMultipleHeadAndTail', testDeleteMultipleHeadAndTail);
    suite.addTest('testDeleteMultipleScattered', testDeleteMultipleScattered);
    suite.addTest('testDeleteAllSameKey', testDeleteAllSameKey);
    suite.addTest('testDeleteMultipleInMiddle', testDeleteMultipleInMiddle);
    suite.addTest('testDeleteMultipleMixed', testDeleteMultipleMixed);
    suite.addTest('testDeleteThenHas', testDeleteThenHas);
    suite.addTest('testDeleteThenGet', testDeleteThenGet);
    suite.addTest('testDeleteThenGetAll', testDeleteThenGetAll);
    suite.addTest('testDeleteThenKeys', testDeleteThenKeys);
    suite.addTest('testDeleteSingleSyncsSearch', testDeleteSingleSyncsSearch);
    suite.addTest('testDeleteMultipleSyncsSearch', testDeleteMultipleSyncsSearch);
    suite.addTest('testDeleteAllSyncsSearch', testDeleteAllSyncsSearch);
    suite.addTest('testDeleteSyncsHref', testDeleteSyncsHref);
    suite.addTest('testDeleteAllSyncsHref', testDeleteAllSyncsHref);
    suite.addTest('testDeleteOnlyParamWithParent', testDeleteOnlyParamWithParent);
    suite.addTest('testDeleteComplexUrl', testDeleteComplexUrl);
    suite.addTest('testDeleteWithHash', testDeleteWithHash);
    suite.addTest('testDeleteSequentialDifferent', testDeleteSequentialDifferent);
    suite.addTest('testDeleteSameTwice', testDeleteSameTwice);
    suite.addTest('testDeleteAllOneByOne', testDeleteAllOneByOne);
    suite.addTest('testDeleteThenAppend', testDeleteThenAppend);
    suite.addTest('testDeleteThenSet', testDeleteThenSet);
    suite.addTest('testDeleteThenAppendWithParent', testDeleteThenAppendWithParent);
    suite.addTest('testDeleteThenSetWithParent', testDeleteThenSetWithParent);
    suite.addTest('testDeleteNonAsciiKey', testDeleteNonAsciiKey);
    suite.addTest('testDeleteEncodedKey', testDeleteEncodedKey);
    suite.addTest('testDeleteFromUrlSearchParams', testDeleteFromUrlSearchParams);
    suite.addTest('testDeleteTwoKeysBothMultiple', testDeleteTwoKeysBothMultiple);

    return suite.run();
}

function testDeleteHead() {
    let params = new url.URLParams('foo=1&bar=2&baz=3');
    params.delete('foo');
    arktest.assertEQ(params.toString(), 'bar=2&baz=3');
}

function testDeleteTail() {
    let params = new url.URLParams('foo=1&bar=2&baz=3');
    params.delete('baz');
    arktest.assertEQ(params.toString(), 'foo=1&bar=2');
}

function testDeleteMiddle() {
    let params = new url.URLParams('foo=1&bar=2&baz=3');
    params.delete('bar');
    arktest.assertEQ(params.toString(), 'foo=1&baz=3');
}

function testDeleteOnlyEntry() {
    let params = new url.URLParams('foo=1');
    params.delete('foo');
    arktest.assertEQ(params.toString(), '');
}

function testDeleteNonExistent() {
    let params = new url.URLParams('foo=1&bar=2');
    params.delete('baz');
    arktest.assertEQ(params.toString(), 'foo=1&bar=2');
}

function testDeleteFromEmpty() {
    let params = new url.URLParams('');
    params.delete('foo');
    arktest.assertEQ(params.toString(), '');
}

function testDeleteMultipleAtHead() {
    let params = new url.URLParams('foo=1&foo=2&bar=3');
    params.delete('foo');
    arktest.assertEQ(params.toString(), 'bar=3');
}

function testDeleteMultipleAtTail() {
    let params = new url.URLParams('bar=1&foo=2&foo=3');
    params.delete('foo');
    arktest.assertEQ(params.toString(), 'bar=1');
}

function testDeleteMultipleHeadAndTail() {
    let params = new url.URLParams('foo=1&bar=2&foo=3');
    params.delete('foo');
    arktest.assertEQ(params.toString(), 'bar=2');
}

function testDeleteMultipleScattered() {
    let params = new url.URLParams('foo=1&bar=2&foo=3&baz=4&foo=5');
    params.delete('foo');
    arktest.assertEQ(params.toString(), 'bar=2&baz=4');
}

function testDeleteAllSameKey() {
    let params = new url.URLParams('foo=1&foo=2&foo=3');
    params.delete('foo');
    arktest.assertEQ(params.toString(), '');
}

function testDeleteMultipleInMiddle() {
    let params = new url.URLParams('bar=1&foo=2&foo=3&baz=4');
    params.delete('foo');
    arktest.assertEQ(params.toString(), 'bar=1&baz=4');
}

function testDeleteMultipleMixed() {
    let params = new url.URLParams('foo=1&foo=2&bar=3&foo=4&baz=5&foo=6');
    params.delete('foo');
    arktest.assertEQ(params.toString(), 'bar=3&baz=5');
}

function testDeleteThenHas() {
    let params = new url.URLParams('foo=1&bar=2');
    params.delete('foo');
    arktest.assertFalse(params.has('foo'));
    arktest.assertTrue(params.has('bar'));
}

function testDeleteThenGet() {
    let params = new url.URLParams('foo=1&bar=2');
    params.delete('foo');
    arktest.assertEQ(params.get('foo'), undefined);
    arktest.assertEQ(params.get('bar'), '2');
}

function testDeleteThenGetAll() {
    let params = new url.URLParams('foo=1&foo=2&bar=3');
    params.delete('foo');
    let result = params.getAll('foo');
    arktest.assertEQ(result.length, 0);
    let barVals = params.getAll('bar');
    arktest.assertEQ(barVals.length, 1);
    arktest.assertEQ(barVals[0], '3');
}

function testDeleteThenKeys() {
    let params = new url.URLParams('foo=1&bar=2&baz=3');
    params.delete('bar');
    let keys: string[] = [];
    for (let k of params.keys()) {
        keys.push(k);
    }
    arktest.assertEQ(keys.length, 2);
    arktest.assertEQ(keys[0], 'foo');
    arktest.assertEQ(keys[1], 'baz');
}

function testDeleteSingleSyncsSearch() {
    let u = url.URL.parseURL('http://localhost?foo=1&bar=2');
    u.params.delete('foo');
    arktest.assertEQ(u.params.toString(), 'bar=2');
    arktest.assertEQ(u.search, '?bar=2');
}

function testDeleteMultipleSyncsSearch() {
    let u = url.URL.parseURL('http://localhost?foo=1&foo=2&foo=3&bar=2');
    u.params.delete('foo');
    arktest.assertEQ(u.params.toString(), 'bar=2');
    arktest.assertEQ(u.search, '?bar=2');
}

function testDeleteAllSyncsSearch() {
    let u = url.URL.parseURL('http://localhost?foo=1&bar=2');
    u.params.delete('foo');
    u.params.delete('bar');
    arktest.assertEQ(u.params.toString(), '');
    arktest.assertEQ(u.search, '');
}

function testDeleteSyncsHref() {
    let u = url.URL.parseURL('http://localhost?foo=1&bar=2');
    u.params.delete('foo');
    arktest.assertEQ(u.href, 'http://localhost/?bar=2');
}

function testDeleteAllSyncsHref() {
    let u = url.URL.parseURL('http://localhost?foo=1&bar=2');
    u.params.delete('foo');
    u.params.delete('bar');
    arktest.assertEQ(u.href, 'http://localhost/');
}

function testDeleteOnlyParamWithParent() {
    let u = url.URL.parseURL('http://localhost?foo=1');
    u.params.delete('foo');
    arktest.assertEQ(u.params.toString(), '');
    arktest.assertEQ(u.search, '');
    arktest.assertEQ(u.href, 'http://localhost/');
}

function testDeleteComplexUrl() {
    let u = url.URL.parseURL('http://user:pass@host:8080/path?foo=1&bar=2#hash');
    u.params.delete('foo');
    arktest.assertEQ(u.search, '?bar=2');
    arktest.assertEQ(u.href, 'http://user:pass@host:8080/path?bar=2#hash');
}

function testDeleteWithHash() {
    let u = url.URL.parseURL('http://localhost?foo=1&bar=2#section');
    u.params.delete('foo');
    arktest.assertEQ(u.search, '?bar=2');
    arktest.assertEQ(u.hash, '#section');
    arktest.assertEQ(u.href, 'http://localhost/?bar=2#section');
}

function testDeleteSequentialDifferent() {
    let params = new url.URLParams('foo=1&bar=2&baz=3');
    params.delete('foo');
    params.delete('baz');
    arktest.assertEQ(params.toString(), 'bar=2');
}

function testDeleteSameTwice() {
    let params = new url.URLParams('foo=1&bar=2');
    params.delete('foo');
    params.delete('foo');
    arktest.assertEQ(params.toString(), 'bar=2');
}

function testDeleteAllOneByOne() {
    let params = new url.URLParams('foo=1&bar=2&baz=3');
    params.delete('foo');
    params.delete('bar');
    params.delete('baz');
    arktest.assertEQ(params.toString(), '');
}

function testDeleteThenAppend() {
    let params = new url.URLParams('foo=1&bar=2');
    params.delete('foo');
    params.append('foo', '3');
    arktest.assertEQ(params.toString(), 'bar=2&foo=3');
}

function testDeleteThenSet() {
    let params = new url.URLParams('foo=1&bar=2');
    params.delete('foo');
    params.set('foo', '3');
    arktest.assertEQ(params.toString(), 'bar=2&foo=3');
}

function testDeleteThenAppendWithParent() {
    let u = url.URL.parseURL('http://localhost?foo=1&bar=2');
    u.params.delete('foo');
    u.params.append('baz', '3');
    arktest.assertEQ(u.search, '?bar=2&baz=3');
    arktest.assertEQ(u.href, 'http://localhost/?bar=2&baz=3');
}

function testDeleteThenSetWithParent() {
    let u = url.URL.parseURL('http://localhost?foo=1&bar=2');
    u.params.delete('foo');
    u.params.set('bar', '9');
    arktest.assertEQ(u.search, '?bar=9');
    arktest.assertEQ(u.href, 'http://localhost/?bar=9');
}

function testDeleteNonAsciiKey() {
    let params = new url.URLParams('foo大=1&bar=2');
    params.delete('foo大');
    arktest.assertEQ(params.toString(), 'bar=2');
}

function testDeleteEncodedKey() {
    let u = url.URL.parseURL('http://localhost?foo%E5%A4%A7=1&bar=2');
    u.params.delete('foo大');
    arktest.assertEQ(u.params.toString(), 'bar=2');
    arktest.assertEQ(u.search, '?bar=2');
}

function testDeleteFromUrlSearchParams() {
    let u = url.URL.parseURL('http://localhost?a=1&b=2&c=3&d=4');
    u.params.delete('b');
    u.params.delete('d');
    arktest.assertEQ(u.params.toString(), 'a=1&c=3');
    arktest.assertEQ(u.search, '?a=1&c=3');
    arktest.assertEQ(u.href, 'http://localhost/?a=1&c=3');
}

function testDeleteTwoKeysBothMultiple() {
    let u = url.URL.parseURL('http://localhost?foo=1&bar=2&foo=3&bar=4&foo=5');
    u.params.delete('foo');
    u.params.delete('bar');
    arktest.assertEQ(u.params.toString(), '');
    arktest.assertEQ(u.search, '');
    arktest.assertEQ(u.href, 'http://localhost/');
}
