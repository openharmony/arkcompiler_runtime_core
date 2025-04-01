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
import {xml} from "api/@ohos.xml.ets"
import {xml} from "api/XmlParseHelper.ets"

function main(): int {
    const suite = new ArkTestsuite("XmlPullParser API tests");

    suite.addTest("Test create XmlPullParser with empty buffer & default Encoding", testXmlPullParserDefault)
    suite.addTest("Test XmlPullParser with TagValueCallback with supportDocType", testWithTagValueCb)
    suite.addTest("Test XmlPullParser with TagValueCallback with ignoreNamespace", testWithTagValueCbWithIgnoreNS)
    suite.addTest("Test XmlPullParser with TagValueCallback with supportDocType and ignoreNamespace", testWithTagValueCbWithSupportDocTypeAndIgnoreNS)
    suite.addTest("Test XmlPullParser with AttributeValueCallback", testWithAttributeValueCb)
    suite.addTest("Test XmlPullParser with AttributeValueCallback with supportDocType and ignoreNamespace", testWithAttributeValueCbWithSupportDocTypeAndIgnoreNS)
    suite.addTest("Test XmlPullParser with TokenValueCallback with supportDocType and ignoreNamespace", testWithTokenValueCb)
    suite.addTest("Test XmlPullParser with TokenValueCallback with supportDocType", testWithTokenValueCbWithNamespace)
    suite.addTest("Test XmlPullParser parse invalid XML", testParseError)
    return suite.run()
}

function testXmlPullParserDefault(){
    let buffer = new ArrayBuffer(0)
    let parser = new xml.XmlPullParser(buffer)
    assertNE(parser, null)
    parser.parse({} as xml.ParseOptions)
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

function processStr(str: string): string {
    return str.replace(new RegExp("[\r\n\s]","g"), "")
}

function testWithTagValueCb() {
    const str : ReadonlyArray<string> = Array.of<string>(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
        "<note importance=\"high\" logged=\"true\">",
        "<![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1\r\n}\r\n]]>",
        "<!--Hello, World!--> <company>John &amp Hans</company> <title>Happy</title>",
        "<title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?>",
        "<a><b/></a> <h:table xmlns:h=\"http://www.w3.org/TR/html4/\"> <h:tr>",
        "<h:td>Apples</h:td> <h:td>Bananas</h:td> </h:tr>",
        "</h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")
    let g_testStr = ''
    let cb = (name: string, value: string): boolean => { 
        g_testStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": false, "tagValueCallbackFunction": cb}

    parser.parse(options)

    const res : ReadonlyArray<string> = Array.of<string>(
        "{  : <?xml verion=\"1.0\" encoding=\"utf-8\"?> }",
        "{  : <?go there?> }",
        "{  : <!--Hello, World!--> }",
        "{  : <!ENTITY foo \"baa\"> }",
        "{  : <!DOCTYPE note [<!ENTITY foo \"baa\">]> }",
        "{  : <![CDATA[function matchwo(a,6){return 1}]]> }",
        "{ company : John &amp Han }",
        "{ title : Happy }",
        "{ title : Happy }",
        "{ todo : Work }",
        "{ todo : Play }",
        "{ b :  }",
        "{ a : <b/> }",
        "{ td : Apple }",
        "{ td : Banana }",
        "{ tr : <h:td>Apple</h:td> <h:td>Banana</h:td>  }",
        "{ table :  <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr> }",
        "{ note : <![CDATA[function matchwo(a,6){return 1}]]><!--Hello, World!--> <company>John &amp Han</company> \
<title>Happy</title><title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?><a><b/></a> \
<h:table xmln:h=\"http://www.w3.org/TR/html4/\"> <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr></h:table> }",
        "{  :  }"
)
    let result = res.join("; ") + "; "
    assertEQ(processStr(g_testStr), processStr(result))
}

function testWithTagValueCbWithIgnoreNS() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
    "<note importance=\"high\" logged=\"true\">",
    "<![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "<!--Hello, World!--> <company>John &amp; Hans</company> <title>Happy</title>",
    "<title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?>",
    "<a><b/></a> <h:table xmlns:h=\"http://www.w3.org/TR/html4/\"> <h:tr>",
    "<h:td>Apples</h:td> <h:td>Bananas</h:td> </h:tr>",
    "</h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")
    let g_testStr = ''
    let cb = (name: string, value: string): boolean => { 
        g_testStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let options: xml.ParseOptions = {"supportDoctype": false, "ignoreNameSpace": true, "tagValueCallbackFunction": cb}

    parser.parse(options)

    const res : ReadonlyArray<string> = Array.of<string>(
        "{  : <?xml verion=\"1.0\" encoding=\"utf-8\"?> }",
        "{  : <?go there?> }",
        "{  : <!--Hello, World!--> }",
        "{  : <!ENTITY foo \"baa\"> }",
        "{  : <![CDATA[function matchwo(a,6){return 1;}]]> }",
        "{ company : John &amp; Han }",
        "{ title : Happy }",
        "{ title : Happy }",
        "{ todo : Work }",
        "{ todo : Play }",
        "{ b :  }",
        "{ a : <b/> }",
        "{ h:td : Apple }",
        "{ h:td : Banana }",
        "{ h:tr : <h:td>Apple</h:td> <h:td>Banana</h:td>  }",
        "{ h:table :  <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr> }",
        "{ note : <![CDATA[function matchwo(a,6){return 1;}]]><!--Hello, World!--> <company>John &amp; Han</company> <title>Happy</title><title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?><a><b/></a> <h:table xmln:h=\"http://www.w3.org/TR/html4/\"> <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr></h:table> }",
        "{  :  }")
    let result = res.join("; ") + "; "
    assertEQ(processStr(g_testStr), processStr(result))
}

function testWithTagValueCbWithSupportDocTypeAndIgnoreNS() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
    "<note importance=\"high\" logged=\"true\">",
    "<![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "<!--Hello, World!--> <company>John &amp; Hans</company> <title>Happy</title>",
    "<title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?>",
    "<a><b/></a> <h:table xmlns:h=\"http://www.w3.org/TR/html4/\"> <h:tr>",
    "<h:td>Apples</h:td> <h:td>Bananas</h:td> </h:tr>",
    "</h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")

    let g_testStr = ''
    let cb = (name: string, value: string): boolean => { 
        g_testStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": true, "tagValueCallbackFunction": cb}

    parser.parse(options)

    const res : ReadonlyArray<string> = Array.of<string>(
        "{  : <?xml verion=\"1.0\" encoding=\"utf-8\"?> }",
        "{  : <?go there?> }",
        "{  : <!--Hello, World!--> }",
        "{  : <!ENTITY foo \"baa\"> }",
        "{  : <!DOCTYPE note [<!ENTITY foo \"baa\">]> }",
        "{  : <![CDATA[function matchwo(a,6){return 1;}]]> }",
        "{ company : John &amp; Hans }", 
        "{ title : Happy }",
        "{ title : Happy }",
        "{ todo : Work }",
        "{ todo : Play }",
        "{ b :  }",
        "{ a : <b/> }",
        "{ h:td : Apple }",
        "{ h:td : Banana }",
        "{ h:tr : <h:td>Apple</h:td> <h:td>Banana</h:td>  }",
        "{ h:table :  <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr> }",
        "{ note : <![CDATA[function matchwo(a,6){return 1;}]]><!--Hello, World!--> <company>John &amp; Han</company> \
<title>Happy</title><title>Happy</title> <todo>Work</todo> <todo>Play</todo> <?go there?><a><b/></a> \
<h:table xmln:h=\"http://www.w3.org/TR/html4/\"> <h:tr><h:td>Apple</h:td> <h:td>Banana</h:td> </h:tr></h:table> }",
        "{  :  }")
    let result = res.join("; ") + "; "
    assertEQ(processStr(g_testStr), processStr(result))
}

function testWithAttributeValueCb() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
    "<note importance=\"high\" logged=\"true\">",
    "    <![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "    <!--Hello, World!-->    <company>John &amp; Hans</company>    <title>Happy</title>",
    "    <title>Happy</title>    <todo>Work</todo>    <todo>Play</todo>    <?go there?>",
    "    <a><b/></a>    <h:table xmlns:h=\"http://www.w3.org/TR/html4/\">        <h:tr>",
    "            <h:td>Apples</h:td>            <h:td>Bananas</h:td>        </h:tr>",
    "    </h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")

    let g_testStr = ''
    let cb = (name: string, value: string): boolean => { 
        g_testStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let options: xml.ParseOptions = {"supportDoctype": false, "ignoreNameSpace": true, "attributeValueCallbackFunction": cb}

    parser.parse(options)

    const expected = "{ 1.0 : verion }; { utf-8 : encoding }; { http://www.w3.org/TR/html4/ : xmlns:h }; { high : importance }; { true : logged }; "
    assertEQ(processStr(g_testStr), processStr(expected))
}

function testWithAttributeValueCbWithSupportDocTypeAndIgnoreNS() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
    "<note importance=\"high\" logged=\"true\">",
    "    <![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "    <!--Hello, World!-->    <company>John &amp; Hans</company>    <title>Happy</title>",
    "    <title>Happy</title>    <todo>Work</todo>    <todo>Play</todo>    <?go there?>",
    "    <a><b/></a>    <h:table xmlns:h=\"http://www.w3.org/TR/html4/\">        <h:tr>",
    "            <h:td>Apples</h:td>            <h:td>Bananas</h:td>        </h:tr>",
    "    </h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")

    let g_testStr = ''
    let cb = (name: string, value: string): boolean => { 
        g_testStr += "{ " + name + " : " + value + " }; "
        return true 
    }

    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": true, "attributeValueCallbackFunction": cb}

    parser.parse(options)

    const expected = "{ 1.0 : verion }; { utf-8 : encoding }; { http://www.w3.org/TR/html4/ : xmlns:h }; { high : importance }; { true : logged }; "
    assertEQ(processStr(g_testStr), processStr(expected));
}

function eventName(type: xml.EventType): string {
    switch (type) {
        case xml.EventType.START_DOCUMENT : return "START_DOCUMENT"
        case xml.EventType.END_DOCUMENT: return "END_DOCUMENT"
        case xml.EventType.START_TAG : return "START_TAG"
        case xml.EventType.END_TAG: return "END_TAG"
        case xml.EventType.TEXT: return "TEXT"
        case xml.EventType.CDSECT: return "CDSECT"
        case xml.EventType.COMMENT: return "COMMENT"
        case xml.EventType.DOCDECL: return "DOCDECL"
        case xml.EventType.INSTRUCTION: return "INSTRUCTION"
        case xml.EventType.ENTITY_REFERENCE: return "ENTITY_REFERENCE"
        case xml.EventType.WHITESPACE: return "WHITESPACE"
        default: return "default"
    }
}

function parseInfo2Str(info: xml.ParseInfo): string {
    let sb = new StringBuilder("")
    sb.append("{ depth : " + info.getDepth() + "}; ")
    sb.append("{ columnNumber : " + info.getColumnNumber()  + "}; ")
    sb.append("{ lineNumber : " + info.getLineNumber()  + "}; ")
    sb.append("{ attributeCount : " + info.getAttributeCount()  + "}; ")
    sb.append("{ isEmptyElementTag : " + info.isEmptyElementTag()  + "}; ")
    sb.append("{ isWhitespace : " + info.isWhitespace()  + "}; ")
    sb.append("{ namespace : " + info.getNamespace()  + "}; ")
    sb.append("{ name : " + info.getName()  + "}; ")
    sb.append("{ prefix : " + info.getPrefix()  + "}; ")
    sb.append("{ text : " + info.getText()  + "}")
    return sb.toString()
}

function testWithTokenValueCb() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance=\"high\" logged=\"true\">",
    "<![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "<!--Hello, World!--> <company>John &amp; Hans</company> <title>Happy</title>",
    "</note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")

    let g_testStr = ''
    let cb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
            g_testStr += "{ " + eventName(eventType) + " : " + parseInfo2Str(value) + " }; "
        return true
    }

    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": true, "tokenValueCallbackFunction": cb}

    parser.parse(options)

    const expected : ReadonlyArray<string> = Array.of<string>( 
    "{ START_DOCUMENT : { depth : 0}; { columnNumber : 0}; { lineNumber : 0}; { attributeCount : 2}; { iEmptyElementTag : fale}; { iWhitepace : fale}; { namepace : }; { name : }; { prefix : }; { text : <?xml verion=\"1.0\" encoding=\"utf-8\"?>} };",
    "{ COMMENT : { depth : 0}; { columnNumber : 3}; { lineNumber : 5}; { attributeCount : 0}; { iEmptyElementTag : fale}; { iWhitepace : fale}; { namepace : }; { name : }; { prefix : }; { text : <!--Hello, World!-->} };",
    "{ CDSECT : { depth : 0}; { columnNumber : 75}; { lineNumber : 0}; { attributeCount : 0}; { iEmptyElementTag : fale}; { iWhitepace : fale}; { namepace : }; { name : }; { prefix : }; { text : <![CDATA[function matchwo(a,6){return 1;}]]>} };",
    "{ START_TAG : { depth : 3}; { columnNumber : 24}; { lineNumber : 5}; { attributeCount : 0}; { iEmptyElementTag : fale}; { iWhitepace : fale}; { namepace : }; { name : company}; { prefix : }; { text : John &amp; Han} };",
    "{ TEXT : { depth : 4}; { columnNumber : 33}; { lineNumber : 5}; { attributeCount : 0}; { iEmptyElementTag : fale}; { iWhitepace : fale}; { namepace : }; { name : company}; { prefix : }; { text : John &amp; Han} };",
    "{ END_TAG : { depth : 3}; { columnNumber : 48}; { lineNumber : 5}; { attributeCount : 0}; { iEmptyElementTag : fale}; { iWhitepace : true}; { namepace : }; { name : /company}; { prefix : }; { text : } };",
    "{ START_TAG : { depth : 3}; { columnNumber : 59}; { lineNumber : 5}; { attributeCount : 0}; { iEmptyElementTag : fale}; { iWhitepace : fale}; { namepace : }; { name : title}; { prefix : }; { text : Happy} };",
    "{ TEXT : { depth : 4}; { columnNumber : 66}; { lineNumber : 5}; { attributeCount : 0}; { iEmptyElementTag : fale}; { iWhitepace : fale}; { namepace : }; { name : title}; { prefix : }; { text : Happy} };",
    "{ END_TAG : { depth : 3}; { columnNumber : 71}; { lineNumber : 5}; { attributeCount : 0}; { iEmptyElementTag : fale}; { iWhitepace : true}; { namepace : }; { name : /title}; { prefix : }; { text : } };",
    "{ START_TAG : { depth : 1}; { columnNumber : 0}; { lineNumber : 0}; { attributeCount : 2}; { iEmptyElementTag : fale}; { iWhitepace : fale}; { namepace : }; { name : note}; { prefix : }; { text : <![CDATA[function matchwo(a,6){return 1;}]]><!--Hello, World!--> <company>John &amp; Han</company> <title>Happy</title>} };",
    "{ TEXT : { depth : 2}; { columnNumber : 36}; { lineNumber : 0}; { attributeCount : 2}; { iEmptyElementTag : fale}; { iWhitepace : fale}; { namepace : }; { name : note}; { prefix : }; { text : <![CDATA[function matchwo(a,6){return 1;}]]><!--Hello, World!--> <company>John &amp; Han</company> <title>Happy</title>} };",
    "{ END_TAG : { depth : 1}; { columnNumber : 40}; { lineNumber : 5}; { attributeCount : 0}; { iEmptyElementTag : fale}; { iWhitepace : true}; { namepace : }; { name : /note}; { prefix : }; { text : } };",
    "{ END_DOCUMENT : { depth : 0}; { columnNumber : 86}; { lineNumber : 5}; { attributeCount : 0}; { iEmptyElementTag : fale}; { iWhitepace : true}; { namepace : }; { name : }; { prefix : }; { text : } }; ")

    assertEQ(processStr(g_testStr), processStr(expected.join(" ")))
}

function testWithTokenValueCbWithNamespace() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
    "<note importance=\"high\" logged=\"true\">",
    "    <![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "    <!--Hello, World!-->    <company>John &amp; Hans</company>    <title>Happy</title>",
    "    <title>Happy</title>    <todo>Work</todo>    <todo>Play</todo>    <?go there?>",
    "    <a><b/></a>    <h:table xmlns:h=\"http://www.w3.org/TR/html4/\">        <h:tr>",
    "            <h:td>Apples</h:td>            <h:td>Bananas</h:td>        </h:tr>",
    "    </h:table></note>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")

    let g_testStr = ''
    let cb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        if (eventType as int === xml.EventType.TEXT as int) {
            g_testStr += "{ " + eventName(eventType) + " : " + " name = " + value.getName() + 
            " ns = " + value.getNamespace() + " pr = " + value.getPrefix() + " }; "
        }
        return true
    }

    let options: xml.ParseOptions = {"supportDoctype": true, "ignoreNameSpace": false, "tokenValueCallbackFunction": cb}

    parser.parse(options)

    const expected : ReadonlyArray<string> = Array.of<string>( 
        "{ TEXT :  name = company n =  pr =  };",
        "{ TEXT :  name = title n =  pr =  };",
        "{ TEXT :  name = title n =  pr =  };",
        "{ TEXT :  name = todo n =  pr =  };",
        "{ TEXT :  name = todo n =  pr =  };",
        "{ TEXT :  name = a n =  pr =  };",
        "{ TEXT :  name = td n = http://www.w3.org/TR/html4/ pr = h };",
        "{ TEXT :  name = td n = http://www.w3.org/TR/html4/ pr = h };",
        "{ TEXT :  name = tr n = http://www.w3.org/TR/html4/ pr = h };",
        "{ TEXT :  name = table n = http://www.w3.org/TR/html4/ pr = h };",
        "{ TEXT :  name = note n =  pr =  }; "
    )
    assertEQ(processStr(g_testStr), processStr(expected.join(" ")))
}

function testParseError() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE note [\n<!ENTITY foo \"baa\">]>",
    "    <![CDATA[\r\nfunction matchwo(a,6)\r\n{\r\nreturn 1;\r\n}\r\n]]>",
    "    <!--Hello, World!-->    <company>John &amp; Hans</company>    <title>Happy</title>",
    "    <title>Happy</title>    <todo>Work</todo>    <todo>Play</todo>    <?go there?>",
    "    <a><b/></a>    <h:table xmlns:h=\"http://www.w3.org/TR/html4/\">        <h:tr>",
    "            <h:td>Apples</h:td>            <h:td>Bananas</h:td>        </h:tr>",
    "    </h:table>")

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)

    let parser = new xml.XmlPullParser(data, "utf-8")
    let assertOccured = false
    try {
        parser.parse({} as xml.ParseOptions)
    }
    catch(e) {
        assertOccured = true
    }
    assertTrue(assertOccured)
}

