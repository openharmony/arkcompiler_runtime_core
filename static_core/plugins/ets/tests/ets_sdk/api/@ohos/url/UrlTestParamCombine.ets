/*
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
import { url } from "@ohos.url";
import { BusinessError } from "@ohos.base";

function testUrlTest102() {
	let params = new url.URLParams();
	params.append("foo~!@#$%^&*()_+-=", "jk");
	assertEQ(params.getAll('jk').length, 0);
	console.log('-------------------------102 done')
}

function testUrlTest103() {
	let params = new url.URLParams();
	params.append("#[%@]", "jk");
	params.append("ma1", "jk1");
	assertEQ(params.getAll('jk').length, 0);
	console.log('-------------------------103 done')
}

function testUrlTest104() {
	let params = new url.URLParams();
	params.append("ma", "jk");
	assertEQ(params.getAll('ma')[0], 'jk');
	console.log('-------------------------104 done')
}

function testUrlTest105() {
	let params = new url.URLParams();
	params.append("你好", "안녕하세요");
	params.append("[saa]", "{aas}");
	assertEQ(params.get('你好'), '안녕하세요');
	console.log('-------------------------105 done')
}

function testUrlTest106() {
	let params = new url.URLParams();
	assertEQ(params.has('你好'), false);
	console.log('-------------------------106 done')
}

function testUrlTest107() {
	let params = new url.URLParams();
	params.append("ma1", "jk1");
	params.set('ma1', 'jk');
	assertEQ(params.get('ma1'), 'jk');
	console.log('-------------------------107 done')
}

function testUrlTest108() {
	let params = new url.URLParams();
	params.append("ma", "jk");
	params.append("foo~!@#$%^&*()_+-=", "jk1");
	params.sort();
	assertEQ(params.toString(), "foo%7E%21%40%23%24%25%5E%26*%28%29_%2B-%3D=jk1&ma=jk");
	console.log('-------------------------108 done')
}

function testUrlTest109() {
	let params = new url.URLParams();
	params.append("ma", "jk");
	params.append("foo~!@#$%^&*()_+-=", "jk1");
	params.append("a", "jk2");
	params.sort();
	assertEQ(params.toString(), "a=jk2&foo%7E%21%40%23%24%25%5E%26*%28%29_%2B-%3D=jk1&ma=jk");
	console.log('-------------------------109 done')
}

function testUrlTest110() {
	let params = new url.URLParams();
	assertEQ(params.has('ma'), false);
	console.log('-------------------------110 done')
}

function testUrlTest111() {
	let params = new url.URLParams();
	params.append("ma", "jk");
	params.append("foo~!@#$%^&*()_+-=", "jk1");
	params.append("a", "jk2");
	let keys = params.keys();
	assertEQ(keys.next().value, "ma");
	assertEQ(keys.next().value, "foo~!@#$%^&*()_+-=");
	assertEQ(keys.next().value, "a");
	console.log('-------------------------111 done')
}

function testUrlTest112() {
	let params = new url.URLParams();
	params.append("ma", "jk");
	params.append("foo~!@#$%^&*()_+-=", "jk1");
	params.append("a", "jk2");
	let values = params.values();
	assertEQ(values.next().value, "jk");
	assertEQ(values.next().value, "jk1");
	assertEQ(values.next().value, "jk2");
	console.log('-------------------------112 done')
}

function testUrlTest113() {
	let params = new url.URLParams();
	params.append("ma", "jk");
	params.append("你好", "jk1");
	params.append("a", "jk2");
	let result = params.entries();
	result.next();
	result.next();
	result.next();
	assertTrue(result.next().value == null);
	console.log('-------------------------113 done')
}

function testUrlTest114() {
	let params = new url.URLParams();
	let result = params.values();
	assertEQ(result.next().done, true);
	console.log('-------------------------114 done')
}

function testUrlTest115() {
	let params = new url.URLParams();
	params.append("你好", "jk1");
	let result = params.values();
	assertEQ(result.next().done, false);
	console.log('-------------------------115 done')
}

function testUrlTest116() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?query=pppppp#123456");
	let params = murl.params;
	let entry = params.entries().next().value!
	assertEQ(entry[0], "query");
	assertEQ(entry[1], "pppppp");
	console.log('-------------------------116 done')
}

function testUrlTest117() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?query=pppppp#123456");
	murl.hash = 'saaf^as\tk\njf'
	assertEQ(murl.hash, "#saaf%5Eas%09k%0Ajf");
	console.log('-------------------------117 done')
}

function testUrlTest118() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?query=pppppp#123456");
	assertEQ(murl.origin, "http://host:8080");
	murl.host = 'salkjf.com';
	assertEQ(murl.origin, "http://salkjf.com:8080");
	console.log('-------------------------118 done')
}

function testUrlTest119() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?query=pppppp#123456");
	assertEQ(murl.pathname, "/directory/file");
	murl.pathname = 'sasf/s?dc#j/mm^h';
	assertEQ(murl.pathname, "/sasf/s?dc#j/mm%5Eh");
	console.log('-------------------------119 done')
}

function testUrlTest120() {
	let murl = url.URL.parseURL("http://username:password@host:8080/directory/file?query=pppppp#123456");
	assertEQ(murl.port, "8080");
	murl.port = '98\t5\r6';
	assertEQ(murl.port, "8080");
	console.log('-------------------------120 done')
}

function testUrlTest121() {
	let murl = url.URL.parseURL("http://username:password@host:443/directory/file?query=pppppp#123456");
	assertEQ(murl.origin, "http://host:443");
	murl.protocol = 'ht\tt\nps:';
	assertEQ(murl.origin, "http://host:443");
	assertEQ(murl.port, "443");
	console.log('-------------------------121 done')
}

function testUrlTest122() {
	let murl = url.URL.parseURL("http://username:password@host:443/directory/file?query=pppppp#123456");
	assertEQ(murl.search, "?query=pppppp");
	murl.search = 'ad/sd=ss`\'c\rc\tc\nc'
	assertEQ(murl.search, "?ad/sd=ss`'c%0Dc%09c%0Ac");
	console.log('-------------------------122 done')
}

function testUrlTest123() {
	let murl = url.URL.parseURL("http://aas@asdgfv.hh:443/index.html?sad=ss中国s");
	assertEQ(murl.toString(), "http://aas@asdgfv.hh:443/index.html?sad=ss%E4%B8%AD%E5%9B%BDs");
	murl.search = 'ad/sd=s中国s`\'cc\tc\rc';
	assertEQ(murl.toString(), "http://aas@asdgfv.hh:443/index.html?ad/sd=s%E4%B8%AD%E5%9B%BDs`'cc%09c%0Dc");
	console.log('-------------------------123 done')
}

function testUrlTest124() {
	let params = new url.URLParams('aa=%E4%B8%AD%E5%9B%BD%BD');
	assertEQ(params.toString(), "aa=%E4%B8%AD%E5%9B%BD%EF%BF%BD")
	assertEQ(params.get('aa'), "中国�")
	console.log('-------------------------124 done')
}

function testUrlTest125() {
	let params = new url.URLParams('aa=%E4%B8%AD%E5%9B%BD%BD中国');
	assertEQ(params.get('aa'), '中国�-�')
	assertEQ(params.toString(), 'aa=%E4%B8%AD%E5%9B%BD%EF%BF%BD-%EF%BF%BD')
}

function testUrlTest126() {
	const objectParams = new url.URLParams('key=a%20b %3D%2Bc++=p%26p%25%E5%A4%A7');
	assertEQ(objectParams.get('key'), 'a b =+c  =p&p%大');
	assertEQ(objectParams.toString(), 'key=a+b+%3D%2Bc++%3Dp%26p%25%E5%A4%A7');

	objectParams.append('key1', 'a%20b%3D%2Bc+=%');
	let res = 'key=a+b+%3D%2Bc++%3Dp%26p%25%E5%A4%A7&key1=a%2520b%253D%252Bc%2B%3D%25';
	assertEQ(objectParams.toString(), res);
	assertEQ(objectParams.get('key1'), 'a%20b%3D%2Bc+=%');
}

function testUrlTest127() {
	let Params2 = new url.URLParams('ssa=s&&&hs==jj&&kk===');
	assertEQ(Params2.toString(), 'ssa=s&hs=%3Djj&kk=%3D%3D');
}

function testUrlTest128() {
	try {
		let murl = url.URL.parseURL("http://username:password@host:199/directory/file?query#fragment");
		murl.href = "saaf^as\tk\njf";
		assertEQ(1, 0)
	} catch (err: BusinessError) {
		assertEQ(err.toString(), 'BusinessError: Syntax Error. Invalid Url string.')
		assertEQ(err.code, 10200002)
		assertEQ(err.message, "Syntax Error. Invalid Url string.");
	}

}

function main(): int {
	let suite = new ArkTestsuite("URL UT tests");

	suite.addTest("testUrlTest102", testUrlTest102);
	suite.addTest("testUrlTest103", testUrlTest103);
	suite.addTest("testUrlTest104", testUrlTest104);
	suite.addTest("testUrlTest105", testUrlTest105);
	suite.addTest("testUrlTest106", testUrlTest106);
	suite.addTest("testUrlTest107", testUrlTest107);
	suite.addTest("testUrlTest108", testUrlTest108);
	suite.addTest("testUrlTest109", testUrlTest109);
	suite.addTest("testUrlTest110", testUrlTest110);
	suite.addTest("testUrlTest111", testUrlTest111);
	suite.addTest("testUrlTest112", testUrlTest112);
	suite.addTest("testUrlTest113", testUrlTest113);
	suite.addTest("testUrlTest114", testUrlTest114);
	suite.addTest("testUrlTest115", testUrlTest115);
	suite.addTest("testUrlTest116", testUrlTest116);
	suite.addTest("testUrlTest117", testUrlTest117);
	suite.addTest("testUrlTest118", testUrlTest118);
	suite.addTest("testUrlTest119", testUrlTest119);
	suite.addTest("testUrlTest120", testUrlTest120);
	suite.addTest("testUrlTest121", testUrlTest121);
	suite.addTest("testUrlTest122", testUrlTest122);
	suite.addTest("testUrlTest123", testUrlTest123);
	suite.addTest("testUrlTest124", testUrlTest124);
	suite.addTest("testUrlTest125", testUrlTest125);
	suite.addTest("testUrlTest126", testUrlTest126);
	suite.addTest("testUrlTest127", testUrlTest127);
	suite.addTest("testUrlTest128", testUrlTest128);

	//passed
	return suite.run();
}
