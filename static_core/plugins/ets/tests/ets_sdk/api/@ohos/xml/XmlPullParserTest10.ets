/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import {xml} from "@ohos.xml"
import { BusinessError } from "@ohos.base";

function main(): int {
    const suite = new ArkTestsuite("XmlPullParser API tests, part10");

    suite.addTest("Testing the XML PullParser without <>.", testXmlPullParserConstructor0020)
    suite.addTest("Testing the XML PullParser with special characters in content. Verifies that it can handle such cases correctly.", testXmlPullParserConstructor0019)
    suite.addTest("Testing the XML PullParser with escaped characters in content. Verifies that it can handle such cases correctly.", testXmlPullParserConstructor0018)
    suite.addTest("Testing the XML PullParser with introduced declaration, parsed correctly.", testXmlPullParserConstructor0017)
    suite.addTest("Testing the XML PullParser with special characters inside elements. Verifies that it can handle such cases correctly.", testXmlPullParserConstructor0016)
    suite.addTest("Testing the XML PullParser with content containing ']]>' within CDATA. Verifies that it handles the input correctly without error.", testXmlPullParserConstructor0015)
    suite.addTest("Testing the XML PullParser with nested CDATA sections.Verifies that it handles nested CDATA sections correctly.", testXmlPullParserConstructor0014)
    suite.addTest("Testing the XML PullParser with a malformed CDATA section. Verifies that it throws a meaningful error for invalid CDATA.", testXmlPullParserConstructor0013)
    suite.addTest("Testing the XML PullParser with names containing special characters.", testXmlPullParserConstructor0008)
    suite.addTest("Testing the XML PullParser with tag names that contain spaces.", testXmlPullParserConstructor0007)

    return suite.run()
}

function rawStringToBuffer( str: string ) : ArrayBuffer {
    let idx = 0
    let len = str.length
    let arr = new Uint8Array( len )
    for ( idx = 0 ; idx < len ; ++idx ) {
        arr[ idx ] = str.charCodeAt(idx) & 0xFF;
    }

    return arr.buffer as ArrayBuffer
}

function testXmlPullParserConstructor0007() {
    const TAG = 'testXmlPullParserConstructor0007';
    try {
        let testXml =
            '<?xml version="1.0" encoding="utf-8" ?>' +
            '<book tag with spaces>' +
            '  <title>XML for Dummies</title>' +
            '  <author>John Doe</author>' +
            '  <price>19.99</price>' +
            '</book>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        assertTrue(false)
    }
}

function testXmlPullParserConstructor0008() {
    const TAG = 'testXmlPullParserConstructor0008';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<book>' +
            '  <ti&>@tle>XML for Dummies</ti-tle>' +
            '  <author>John Doe</author>' +
            '  <price>19.99</price>' +
            '  <description>This book is about XML.</description>' +
            '</book>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        assertTrue(false)
    } catch (err: BusinessError) {
        console.error(`${TAG} failed, error: ${err}`);
        assertEQ((err as BusinessError).code, 401);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
    }
}

function testXmlPullParserConstructor0013() {
    const TAG = 'testXmlPullParserConstructor0013';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<!--注释的内容-->' +
            '<students>' +
            '    <!--第一个学生信息-->' +
            '    <student id="1">' +
            '        <name>张三</name>' +
            '        <age>23</age>' +
            '        <info>学生&lt; &gt;&gt;&gt;&gt;的信息</info>' +
            '        <message> <!%[CDATA[内容 <<<<<< >>>>>> ]]]></message>' +
            '    </student>' +
            '    <body>I was parsed!</body>' +
            '</students>'
        
        
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        assertTrue(false)
    } catch (err: BusinessError) {
        console.error(`${TAG} failed, error: ${err}`);
        assertEQ((err as BusinessError).code, 401);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
    }
}

function testXmlPullParserConstructor0014() {
    const TAG = 'testXmlPullParserConstructor0014';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<!--注释的内容-->' +
            '<students>' +
            '    <!--第一个学生信息-->' +
            '    <student id="1">' +
            '        <name>张三</name>' +
            '        <age>23</age>' +
            '        <info>学生&lt; &gt;&gt;&gt;&gt;的信息</info>' +
            '        <message> <![CDATA[内容<![CDATA[嵌套 ]]> ]]></message>' +
            '    </student>' +
            '    <body>I was parsed!</body>' +
            '</students>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        assertTrue(false)
    } catch (err: BusinessError) {
        console.error(`${TAG} failed, error: ${err}`);
        assertEQ((err as BusinessError).code, 401);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
    }
}

function testXmlPullParserConstructor0015() {
    const TAG = 'testXmlPullParserConstructor0015';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<!--注释的内容-->' +
            '<students>' +
            '    <!--第一个学生信息-->' +
            '    <student id="1">' +
            '        <name>张三</name>' +
            '        <age>23</age>' +
            '        <info>学生&lt; &gt;&gt;&gt;&gt;的信息</info>' +
            '        <message> <![CDATA[内容都发给对方]]>分隔符发 ]]></message>' +
            '    </student>' +
            '    <body>I was parsed!</body>' +
            '</students>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        assertTrue(false)
    } catch (err: BusinessError) {
        console.error(`${TAG} failed, error: ${err}`);
        assertEQ((err as BusinessError).code, 401);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
    }
}

function testXmlPullParserConstructor0016() {
    const TAG = 'testXmlPullParserConstructor0016';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<!--注释的内容-->' +
            '<students>' +
            '    <!--第一个学生信息-->' +
            '    <student id="1">' +
            '        <name>张三</name>' +
            '        <age>23</age>' +
            '        <info>学生&lt; &gt;&gt;&gt;&gt;的信息</info>' +
            '        <message> <![CDATA[内容%$^# ]]></message>' +
            '    </student>' +
            '    <body>I was parsed!</body>' +
            '</students>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        assertTrue(false)
    }
}

function testXmlPullParserConstructor0017() {
    const TAG = 'testXmlPullParserConstructor0017';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<?xml-stylesheet type="text/css" href="cd_catalog.css"?>' +
            '<CATALOG>' +
            '<CD>' +
            '<TITLE>Empire Burlesque</TITLE>' +
            '<ARTIST>Bob Dylan</ARTIST>' +
            '<COUNTRY>USA</COUNTRY>' +
            '<COMPANY>Columbia</COMPANY>' +
            '<PRICE>10.90</PRICE>' +
            '<YEAR>1985</YEAR>' +
            '</CD>' +
            '</CATALOG>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        assertTrue(false)
    }
}

function testXmlPullParserConstructor0018() {
    const TAG = 'testXmlPullParserConstructor0018';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<NOTE>' +
            '<TO>John</TO>' +
            '<FROM>Jane</FROM>' +
            '<MESSAGE>Hello & Welcome to the \"XML\" World!</MESSAGE>' +
            '</NOTE>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        assertTrue(false)
    } catch (err: BusinessError) {
        console.error(`${TAG} failed, error: ${err}`);
        assertEQ((err as BusinessError).code, 401);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
    }
}

function testXmlPullParserConstructor0019() {
    const TAG = 'testXmlPullParserConstructor0019';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<!-- This is a comment -->' +
            '<root><>&<root>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        assertTrue(false)
    } catch (err: BusinessError) {
        console.error(`${TAG} failed, error: ${err}`);
        assertEQ((err as BusinessError).code, 401);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
    }
}

function testXmlPullParserConstructor0020() {
    const TAG = 'testXmlPullParserConstructor0020';
    try {
        let testXml =
            'xml version="1.0" encoding="utf-8"' +
            '<note importance="high" logged="true">' +
            ' <title>Happy</title>' +
            ' <todo>Work</todo>' +
            ' <todo>Play</todo>' +
            '</note>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        assertTrue(false)
    } catch (err: BusinessError) {
        console.error(`${TAG} failed, error: ${err}`);
        assertEQ((err as BusinessError).code, 401);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
    }
}

