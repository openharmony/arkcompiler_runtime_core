/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import {xml} from '@ohos.xml';
import {stream} from '@ohos.util.stream';

function main(): int {
    let suite = new arktest.ArkTestsuite('XML SAX UT tests');

    suite.addAsyncTest('XmlSAXParserConstructorTest0001', XmlSAXParserConstructorTest0001);
    suite.addAsyncTest('XmlSAXParserConstructorTest0002', XmlSAXParserConstructorTest0002);
    suite.addAsyncTest('XmlSAXParserConstructorTest0003', XmlSAXParserConstructorTest0003);
    suite.addAsyncTest('XmlSAXParserConstructorTest0004', XmlSAXParserConstructorTest0004);
    suite.addAsyncTest('XmlSAXParserConstructorTest0005', XmlSAXParserConstructorTest0005);
    return suite.run();
}

class TestReadable extends stream.Readable {
    constructor() {
      super();
    }

    doRead(size: int) {
    }
}

async function XmlSAXParserConstructorTest0001(): Promise<void> {
    let startDocumentCalled = false;
    let endDocumentCalled = false;
    let startElementCalled = false;
    let endElementCalled = false;
    let charactersCalled = false;

    let parseComplete: (value: undefined | PromiseLike<undefined>) => void = ()=> {};
    const parsePromise = new Promise<void>((resolve) => {
        parseComplete = resolve;
    });

    const saxHandler: xml.XmlSAXHandler = {
      startDocument() {
        startDocumentCalled = true;
      },
      endDocument() {
        endDocumentCalled = true;
        parseComplete(undefined);
      },
      startElement(elementName: string, namespaceURI?: string, qName?: string, attributes?: Map<string, string>) {
        startElementCalled = true;
      },
      endElement(elementName: string, namespaceURI?: string, qName?: string) {
        endElementCalled = true;
      },
      characters(content: string) {
        charactersCalled = true;
      }
    };

    const testData = '<?xml version="1.0" encoding="UTF-8"?><root><child>Text</child></root>';
    const inputStream = new TestReadable();
    const saxParser = new xml.XmlSAXParser(inputStream);

    saxParser.parse(saxHandler);
    inputStream.push(testData);
    inputStream.push(null);

    await parsePromise;

    arktest.assertTrue(startDocumentCalled)
    arktest.assertTrue(endDocumentCalled)
    arktest.assertTrue(startElementCalled)
    arktest.assertTrue(endElementCalled)
    arktest.assertTrue(charactersCalled)
}

async function XmlSAXParserConstructorTest0002(): Promise<void> {
    let receivedElementName : string | undefined = '';
    let receivedNamespaceURI : string | undefined = '';
    let receivedQName : string | undefined = '';
    let receivedAttributes: Map<string, string> | undefined;

    let parseComplete: (value: undefined | PromiseLike<undefined>) => void = ()=> {};
    const parsePromise = new Promise<void>((resolve) => {
        parseComplete = resolve;
    });

    const saxHandler: xml.XmlSAXHandler = {
      startDocument() {
      },
      endDocument() {
        parseComplete(undefined);
      },
      startElement(elementName: string, namespaceURI?: string, qName?: string, attributes?: Map<string, string>) {
        receivedElementName = elementName;
        receivedNamespaceURI = namespaceURI;
        receivedQName = qName;
        receivedAttributes = attributes;
      },
      endElement(elementName: string, namespaceURI?: string, qName?: string) {
      },
      characters(content: string) {
      }
    };

    const testData = '<?xml version="1.0" encoding="UTF-8"?>' +
        '<root xmlns:ns1="http://example.com/ns1">' +
        '<ns1:child ns1:attr1="value1" attr2="value2">Text</ns1:child></root>';
    const inputStream = new TestReadable();
    const saxParser = new xml.XmlSAXParser(inputStream);

    saxParser.parse(saxHandler);
    inputStream.push(testData);
    inputStream.push(null);

    await parsePromise;

    arktest.assertEQ(receivedElementName, 'child')
    arktest.assertEQ(receivedNamespaceURI, 'http://example.com/ns1')
    arktest.assertEQ(receivedQName, 'ns1:child')
    arktest.assertTrue(receivedAttributes instanceof Map)
}

async function XmlSAXParserConstructorTest0003(): Promise<void> {
    let elementCount = 0;
    const elementNames: string[] = [];

    let parseComplete: (value: undefined | PromiseLike<undefined>) => void = ()=> {};
    const parsePromise = new Promise<void>((resolve) => {
        parseComplete = resolve;
    });

    const saxHandler: xml.XmlSAXHandler = {
        startDocument() {
        },
        endDocument() {
            parseComplete(undefined);
        },
        startElement(elementName: string, namespaceURI?: string, qName?: string, attributes?: Map<string, string>) {
          elementCount++;
          elementNames.push(elementName);
        },
        endElement(elementName: string, namespaceURI?: string, qName?: string) {
        },
        characters(content: string) {
        }
      };

      const testData = '<?xml version="1.0" encoding="UTF-8"?>' +
          '<root><item1>Text1</item1><item2>Text2</item2><item3>Text3</item3></root>';
      const inputStream = new TestReadable();
      const saxParser = new xml.XmlSAXParser(inputStream);

      saxParser.parse(saxHandler);
      inputStream.push(testData);
      inputStream.push(null);

    await parsePromise;

    arktest.assertEQ(elementCount, 4)
    arktest.assertTrue(elementNames.includes('root'))
    arktest.assertTrue(elementNames.includes('item1'))
    arktest.assertTrue(elementNames.includes('item2'))
    arktest.assertTrue(elementNames.includes('item3'))
}

async function XmlSAXParserConstructorTest0004(): Promise<void> {
    let attributesReceived = false;
    let attrValue1 : string | undefined = '';
    let attrValue2 : string | undefined = '';

    let parseComplete: (value: undefined | PromiseLike<undefined>) => void = ()=> {};
    const parsePromise = new Promise<void>((resolve) => {
        parseComplete = resolve;
    });

    const saxHandler: xml.XmlSAXHandler = {
        startDocument() {
        },
        endDocument() {
            parseComplete(undefined);
        },
        startElement(elementName: string, namespaceURI?: string, qName?: string, attributes?: Map<string, string>) {
          if (attributes && attributes.size > 0) {
            attributesReceived = true;
            attrValue1 = attributes.get('id') || '';
            attrValue2 = attributes.get('name') || '';
          }
        },
        endElement(elementName: string, namespaceURI?: string, qName?: string) {
        },
        characters(content: string) {
        }
      };

      const testData = '<?xml version="1.0" encoding="UTF-8"?><root id="123" name="test">Content</root>';
      const inputStream = new TestReadable();
      const saxParser = new xml.XmlSAXParser(inputStream);

      saxParser.parse(saxHandler);
      inputStream.push(testData);
      inputStream.push(null);

    await parsePromise;

    arktest.assertTrue(attributesReceived)
    arktest.assertEQ(attrValue1, '123')
    arktest.assertEQ(attrValue2, 'test')
}

async function XmlSAXParserConstructorTest0005(): Promise<void> {
  let maxDepth = 0;
  let currentDepth = 0;

  let parseComplete: (value: undefined | PromiseLike<undefined>) => void = ()=> {};
  const parsePromise = new Promise<void>((resolve) => {
      parseComplete = resolve;
  });

  const saxHandler: xml.XmlSAXHandler = {
    startDocument() {
    },
    endDocument() {
        parseComplete(undefined);
    },
    startElement(elementName: string, namespaceURI?: string, qName?: string, attributes?: Map<string, string>) {
      currentDepth++;
      if (currentDepth > maxDepth) {
        maxDepth = currentDepth;
      }
    },
    endElement(elementName: string, namespaceURI?: string, qName?: string) {
      currentDepth--;
    },
    characters(content: string) {
    }
  };

  const testData = '<?xml version="1.0" encoding="UTF-8"?><a><b><c><d><e>Deep</e></d></c></b></a>';
  const inputStream = new TestReadable();
  const saxParser = new xml.XmlSAXParser(inputStream);

  saxParser.parse(saxHandler);
  inputStream.push(testData);
  inputStream.push(null);

  await parsePromise;

  arktest.assertEQ(maxDepth, 5)
}
