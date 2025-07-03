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
import buffer from '@ohos.buffer';
import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: number = 401;
const BufferOverFlowErrorCodeId: number = 10200062;
const IllegalPositionErrorCodeId: number = 10200063;
const EmptyErrorCodeId: number = 10200064;
const NoElementMatchErrorCodeId: number = 10200065;
const EncodingErrorCodeId: number = 10200066;

function createBusinessError(code: number, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}

export namespace xml {

    export enum EventType {
        START_DOCUMENT,
        END_DOCUMENT,
        START_TAG,
        END_TAG,
        TEXT,
        CDSECT,
        COMMENT,
        DOCDECL,
        INSTRUCTION,
        ENTITY_REFERENCE,
        WHITESPACE
    }

    export class ParseInfoImpl implements ParseInfo {
        depth: number
        columnNumber: number
        lineNumber: number
        attributeCount: number
        name: string
        namespace_: string
        prefix: string
        text: string
        emptyElementTag: boolean
        whitespace: boolean

        override getAttributeCount(): number { return this.attributeCount; }
        override getColumnNumber(): number { return this.columnNumber; }
        override getDepth(): number { return this.depth; }
        override getLineNumber(): number { return this.lineNumber; }
        override getName(): string { return this.name; }
        override getNamespace(): string { return this.namespace_; }
        override getPrefix(): string { return this.prefix; }
        override getText(): string { return this.text; }
        override isEmptyElementTag(): boolean { return this.emptyElementTag; }
        override isWhitespace(): boolean { return this.whitespace; }
    }

    export interface ParseInfo {
        getAttributeCount(): number
        getColumnNumber(): number
        getDepth(): number
        getLineNumber(): number
        getName(): string
        getNamespace(): string
        getPrefix(): string
        getText(): string
        isEmptyElementTag(): boolean
        isWhitespace(): boolean
    }

    export interface ParseOptions {
        attributeValueCallbackFunction?: (name: string, value: string) => boolean
        ignoreNameSpace?: boolean
        supportDoctype?: boolean
        tagValueCallbackFunction?: (name: string, value: string) => boolean
        tokenValueCallbackFunction?: (eventType: EventType, value: ParseInfo) => boolean
    }

    export class XmlPullParser {

        constructor(bufferIn: ArrayBuffer | DataView, encoding?: string) {
            let strIn: string
            let b: buffer.Buffer
            if (bufferIn instanceof DataView) {
                b = buffer.from((bufferIn as DataView).buffer as ArrayBuffer)
            }
            else {
                b = buffer.from(bufferIn)
            }
            strIn = (encoding != undefined) ? b.toString(encoding! as buffer.BufferEncoding) : b.toString()
            this.helper = new XmlParseHelper(strIn)
        }

        private helper: XmlParseHelper
        public parseXml(option: ParseOptions): void {
            this.helper.supportDoctype = (option.supportDoctype != undefined) ? option.supportDoctype! : false
            this.helper.ignoreNameSpace = (option.ignoreNameSpace != undefined) ? option.ignoreNameSpace! : false
            this.helper.tagFunc_ = option.tagValueCallbackFunction
            this.helper.attrFunc_ = option.attributeValueCallbackFunction
            this.helper.tokenFunc_ = option.tokenValueCallbackFunction
            this.helper.parseXml()
        }
    }
    
    type Attributes = Map<string, string>

    interface XmlNode {
        name?: string
        attributes?: Attributes
        content?: string
        lineNumber?: number
        columnNumber?: number
        pos?: number
        depth?: number
        prefix?: string
        namespace_?: string
        type?: EventType
        isSelfClose?: boolean
        isServiceTagNode?: boolean
    }

    export const MaxXmlLength = 100000;
    const OffsetDepth = 1;
    const SpacesIndentation = 2;

    enum XmlDynamicType {
        NONE,
        START_AND_ATTRIBUTES,
        DECLARATION,
        TEXT,
        OTHERS
    }

    export class XmlDynamicSerializer {
        private iPos: int = 0;
        private buffer: ArrayBuffer;
        private encoding: string = 'utf-8';
        private depth: int = 0;
        private type = XmlDynamicType.NONE;
        private elementStack = new Array<XmlNode>();
        private multNsp = new Map<int, XmlNode>();
        private elementNum: int = 0;

        /**
         * Constructs an instance of the class with the specified buffer and encoding.
         *
         * @param {string} encoding - The encoding format of the data. Only 'utf-8' is supported.
         * @throws {BusinessError} Throws an error if the encoding is not 'utf-8'.
         */
        constructor(encoding: string) {
            if (encoding !== 'utf-8') {
                throw new BusinessError(EncodingErrorCodeId, new Error('Parameter error.Just support utf-8'));
            }

            this.buffer = new ArrayBuffer(MaxXmlLength);
            this.increaseStackLength();
        }

        /**
         * Constructs an instance of the class with the specified buffer.
         */
        constructor() {
            this.buffer = new ArrayBuffer(MaxXmlLength);
            this.increaseStackLength();
        }

        /**
         * Sets an attribute for the current XML element.
         *
         * @param {string} name - The name of the attribute to set.
         * @param {string} value - The value of the attribute to set.
         * @throws {BusinessError} Throws an error if the method is called in an illegal position and
         * the `name` parameter is an empty string.
         */
        public setAttributes(name: string, value: string): void {
            this.checkEmptyParameter(name);
            if (this.type !== XmlDynamicType.START_AND_ATTRIBUTES) {
                this.checkPosition();
            }

            let temp = new StringBuilder(` ${name}=\"`);
            this.writeEscaped(temp, value);
            temp.append(`\"`);
            this.type = XmlDynamicType.START_AND_ATTRIBUTES;
            this.writeToBuffer(temp);
        }

        /**
         * Sets the XML declaration for the current document.
         *
         * @throws {BusinessError} Throws an error if the declaration is set in an illegal position.
         */
        public setDeclaration(): void {
            if (this.iPos > 0) {
                this.checkPosition();
            }

            let temp = new StringBuilder(`<?xml version=\"1.0\" encoding=\"${this.encoding}\"?>`);
            this.type = XmlDynamicType.DECLARATION;
            this.writeToBuffer(temp);
        }

        /**
         * Handles the start of an XML element.
         *
         * @param {string} name - The name of the XML element being started.
         * @throws {BusinessError} Throws an error if the `name` parameter is an empty string.
         */
        public startElement(name: string): void {
            this.checkEmptyParameter(name);
            let temp = new StringBuilder();
            if (this.type === XmlDynamicType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(temp);
                temp.append('>');
            }
            if (this.type !== XmlDynamicType.NONE) {
                this.nextItem(temp);
            }

            this.elementStack[this.depth].name = name;
            temp.append('<');
            if (this.elementStack[this.depth].prefix !== '') {
                temp.append(`${this.elementStack[this.depth].prefix}:`);
            } else if (this.depth !== 0) {
                if (this.elementStack[this.depth - OffsetDepth].prefix !== '') {
                    this.elementStack[this.depth].prefix = this.elementStack[this.depth - OffsetDepth].prefix;
                    temp.append(`${this.elementStack[this.depth].prefix}:`);
                }
            }
            temp.append(`${this.elementStack[this.depth].name}`);
            this.type = XmlDynamicType.START_AND_ATTRIBUTES;
            this.depth++;
            this.elementNum++;
            this.increaseStackLength();
            this.writeToBuffer(temp);
        }

        /**
         * Handles the end of an XML element during parsing.
         *
         * @throws {BusinessError} Throws an error if there is no matching start element for the current end element.
         */
        public endElement(): void {
            if (this.elementNum < 1) {
                throw new BusinessError(NoElementMatchErrorCodeId,
                    new Error('There is no match between the startElement and the endElement'));
            }
            this.elementNum--;

            let temp = new StringBuilder();
            if (this.type == XmlDynamicType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(temp);
                temp.append('/>');
                this.type = XmlDynamicType.OTHERS;
                this.depth--;
                this.writeToBuffer(temp);
                return;
            }
            this.depth--;
            if (this.type !== XmlDynamicType.TEXT) {
                this.nextItem(temp);
            }

            temp.append('</');
            if (this.elementStack[this.depth].prefix !== '') {
                temp.append(`${this.elementStack[this.depth].prefix}:`);
            }
            temp.append(`${this.elementStack[this.depth].name}`);
            this.elementStack[this.depth].prefix = '';
            this.elementStack[this.depth].namespace_ = '';
            this.type = XmlDynamicType.OTHERS;
            temp.append('>');
            this.writeToBuffer(temp);
        }

        /**
         * Sets the namespace for the current XML element.
         *
         * @param {string} prefix - The namespace prefix to be set.
         * @param {string} namespace - The namespace URI associated with the prefix.
         * @throws {BusinessError} Throws an error if the `prefix` or `name` parameter is an empty string.
         */
        public setNamespace(prefix: string, namespace: string): void {
            this.checkEmptyParameter(prefix);
            this.checkEmptyParameter(namespace);

            let temp = new StringBuilder();
            if (this.type === XmlDynamicType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(temp);
                temp.append('>');
            }

            this.elementStack[this.depth].prefix = prefix;
            this.elementStack[this.depth].namespace_ = namespace;
            let node: XmlNode = {
                prefix: this.elementStack[this.depth]!.prefix,
                namespace_: this.elementStack[this.depth]!.namespace_
            }
            this.multNsp.set(this.depth, node);
            this.type = XmlDynamicType.OTHERS;
            this.writeToBuffer(temp);
        }

        /**
         * Retrieves a copy of the internal buffer as an ArrayBuffer.
         *
         * @returns {ArrayBuffer} A new ArrayBuffer containing the data from the internal buffer.
         */
        public getOutput(): ArrayBuffer {
            return this.buffer.slice(0, this.iPos);
        }

        /**
         * Increases the length of the element stack by creating a new `XmlNode` object with default empty values for
         * `name`, `prefix`, and `namespace_`, and pushing it onto the `elementStack`.
         */
        private increaseStackLength(): void {
            let node: XmlNode = {
                name: '',
                prefix: '',
                namespace_: ''
            }
            this.elementStack.push(node);
        }

        /**
         * Escapes special characters in the given string and appends the result to the provided StringBuilder.
         *
         * @param {StringBuilder} stringBuilder - The StringBuilder to which the escaped string will be appended.
         * @param {string} s - The input string to be escaped.
         */
        private writeEscaped(stringBuilder: StringBuilder, s: string): void {
            s = s.replaceAll('&', '&amp;');
            s = s.replaceAll('\'', '&apos;');
            s = s.replaceAll('\"', '&quot;');
            s = s.replaceAll('>', '&gt;');
            s = s.replaceAll('<', '&lt;');
            stringBuilder.append(s);
        }

        /**
         * Writes the content of the `stringBuilder` array to the `buffer` starting at the current position (`iPos`).
         * If the buffer does not have enough space to accommodate the data, an error is thrown.
         *
         * @param {StringBuilder} stringBuilder - The StringBuilder will be write.
         * @param {string} name - The name used in the error message if the operation fails.
         * @throws {BusinessError} Throws an error if the buffer does not have enough space to write the data.
         */
        private writeToBuffer(stringBuilder: StringBuilder): void {
            let tempString = stringBuilder.toString();
            let iLenTemp = tempString.length;
            if (this.iPos + iLenTemp <= MaxXmlLength) {
                for (let i = 0; i < iLenTemp; i++) {
                    this.buffer.set((i + this.iPos) as int, tempString[i] as byte);
                }
                this.iPos += iLenTemp;
            } else {
                throw new BusinessError(BufferOverFlowErrorCodeId, new Error('The length has exceeded the upper limit'));
            }
        }

        /**
         * Handles the namespace processing for the current element in the stack.
         *
         * @param {StringBuilder} stringBuilder - The `StringBuilder` instance used to append
         * namespace declarations to the XML output.
         */
        private spliceNamespace(stringBuilder: StringBuilder): void {
            if (this.depth === 0) {
                return;
            }

            this.elementStack[this.depth].prefix = this.elementStack[this.depth - OffsetDepth].prefix;
            this.elementStack[this.depth].namespace_ = this.elementStack[this.depth - OffsetDepth].namespace_;

            let prevDepthInfo = this.multNsp.get(this.depth - OffsetDepth);
            if (prevDepthInfo == undefined) {
                return;
            }

            if (this.type === XmlDynamicType.START_AND_ATTRIBUTES) {
                let prefix = prevDepthInfo.prefix;
                let namespace = prevDepthInfo.namespace_;
                stringBuilder.append(` xmlns:${prefix}=\"${namespace}\"`);
                this.multNsp.get(this.depth - OffsetDepth)!.prefix = '';
                this.multNsp.get(this.depth - OffsetDepth)!.namespace_ = '';
            }
        }

        /**
         * Appends a newline character followed by indentation spaces to the `out` property.
         *
         * @param {StringBuilder} stringBuilder - The `StringBuilder` instance used to append spaces to the XML output.
         */
        private nextItem(stringBuilder: StringBuilder): void {
            stringBuilder.append('\r\n');
            stringBuilder.append(' '.repeat(SpacesIndentation * this.depth));
        }

        /**
         * Checks if the provided parameter is an empty string.
         * Throws a `BusinessError` if the parameter is empty.
         *
         * @param {string} para - The parameter to be checked.
         */
        private checkEmptyParameter(para: string): void {
            if (para.length === 0) {
                throw new BusinessError(EmptyErrorCodeId, new Error('Parameter error. Parameter cannot be empty'));
            }
        }

        /**
         * Checks the position of the XML serializer and throws an error if the position is illegal.
         */
        private checkPosition(): void {
            throw new BusinessError(IllegalPositionErrorCodeId, new Error('Illegal position for xml'));
        }
    }

    export class XmlParseHelper {

        public supportDoctype: boolean = false
        public ignoreNameSpace: boolean = false
        public tagFunc_?: (name: string, value: string) => boolean
        public attrFunc_?: (name: string, value: string) => boolean
        public tokenFunc_?: (eventType: EventType, value: ParseInfo) => boolean

        private strXml_ : string = " "
        private nodeMap = new Map<number, XmlNode>(); 

        constructor(str: string) {
            this.strXml_ = str
        }

        private static readonly XML = "<?xml"
        private static readonly SEMICOLON = ':'
        private static readonly HTTP = 'http'
        private static readonly HTTPS = 'https'
        private static readonly UNDESCORE = '_'
        private static readonly XMLNS = 'xmlns'
        private static readonly XMLNS_PREFIX = 'xmlns:'
        private static readonly lineEndRegExp = new RegExp("\n", "m")
        private static readonly selfCloseTagRegExp = new RegExp("<([\\w:]+)([^<^>])*?/>", "m")
        private static readonly startTagRegExp = new RegExp("<([\\w:]+)[\\s\\S]*?>", "m")
        private static readonly closeTagRegExp = new RegExp("</([\\w:]+)([^<^>])*?>", "m")
        private static readonly textTagRegExp = new RegExp(">[&|\\w:|\\s][^<>]+<", "m")
        private static readonly attributeRegExp = new RegExp("([\\w:-]+)\\s*=\\s*(\"[^\"]*\"|'[^']*'|\\w+)\\s*", "m")
        private static readonly stripRegExp = new RegExp("^['\"]|['\"]$","g")
        private static readonly entityRegExp = new RegExp("<!ENTITY (?<name>\\w+) ('|\")(?<value>.*?)('|\")>", "gm")
        private static readonly nodeNameRegExp = new RegExp("^[\\w:]+$", "gm")

        private static readonly serviceTagRegExpMap = new Map<EventType, RegExp>([
            [EventType.INSTRUCTION, new RegExp("<\\?[^>]+?>","g")],
            [EventType.COMMENT, new RegExp("<!--[\\s\\S]*?-->","g")],
            [EventType.ENTITY_REFERENCE, new RegExp("<!ENTITY[\\s\\S]*?>","g")],
            [EventType.DOCDECL, new RegExp("<!DOCTYPE[\\s\\S]*?\[[\\s\\S]*?\]>","g")],
            [EventType.CDSECT, new RegExp("<!\\[CDATA\\[[\\s\\S]*?\]\]>","gm")]
        ])

        private static readonly xmlEscapeChars = new Map<string, string>([
            ["&quot;", "\""],
            ["&amp;", "&"],
            ["&lt;", "<"],
            ["&gt;", ">"],
            ["&apos;", "'"]
        ])

        private replaceEntities(node: XmlNode): XmlNode {
            let str = (node.content != undefined) ? node.content! : ""
            if (str == "") {
                return node
            }
            if (this.supportDoctype && str.includes("&")) {
                this.documentEntities.forEach((value: string, key: string) => {
                    let entityRef = new StringBuilder().append("&").append(key).toString()
                    if (str.includes(entityRef)) {
                        str = str.replace(entityRef, value)
                        node.type = EventType.ENTITY_REFERENCE
                        node.name = key
                        node.content = str
                    }
                })
            }
            return node
        }

        private replaceXmlSpecChars(str: string): string {
            for (let key of XmlParseHelper.xmlEscapeChars.keys()) {
                str = str.replace(key, XmlParseHelper.xmlEscapeChars.get(key!) as string)
            }

            return str
        }

        private removeSpecTags(str: string): string {
            if (str.includes("CDATA")) {
                str = str.replace("<![CDATA[", "").replace("]]>", "")
            }
            if (str.includes("<!--")) {
                str = str.replace("<!--", "").replace("-->", "")
            }
            if (str.includes("<?")) {
                str = str.replace("<?", "").replace("?>", "")
            }
            if (str.includes("<!ENTITY")) {
                str = str.replace("<!ENTITY","").replace(">", "")
            }
            if (str.includes("<!DOCTYPE")) {
                str = str.replace("<!DOCTYPE","").replace(">", "")
            }
            return str
        }

        public calcPosition(str: string, node: XmlNode) {
            let lineEndMatch: NullishType
            let pos: number = 1
            let end = str.length > 1 ? str.length + 1 : 1
            let lineChanged: boolean = false
            node.lineNumber = 1
            node.columnNumber = 1
            node.pos = end
            while ((pos < end) && (lineEndMatch = XmlParseHelper.lineEndRegExp.exec(str.substr(pos)))) {
                node.lineNumber!++
                pos += lineEndMatch!.index + 1
                lineChanged = true
            }

            if (lineChanged) {
                node.columnNumber = (abs(end - pos) + 1)
            } else {
                node.columnNumber = end
            }
        }

        private parseToken(node: XmlNode): ParseInfo {

            let info = new ParseInfoImpl()
            info.depth = (node.depth != undefined) ? node.depth! : 0
            info.columnNumber = (node.columnNumber != undefined) ? node.columnNumber! : 0
            info.lineNumber = (node.lineNumber != undefined) ? node.lineNumber! : 0
            info.attributeCount = (node.attributes != undefined) ? node.attributes!.size : 0
            info.emptyElementTag = (node.isSelfClose != undefined) ? (node.isSelfClose!) : false
            info.whitespace = (node.content != undefined) ? (node.content! == " " || node.content! == "") : true
            info.namespace_ = (node.namespace_ != undefined) ? node.namespace_! : ""
            info.name = (node.name != undefined) ? node.name! : ""
            info.prefix = (node.prefix != undefined) ? node.prefix! : ""
            info.text = (node.content != undefined) ? this.replaceXmlSpecChars(node.content!) : ""

            return info
        }
        private startDocumentFound: boolean = false
        private documentEntities = new Map<string,string>()
        private processServiceTag(regexp: RegExp, type: EventType) {
            let m : NullishType
            let pos: number = 0
            while (m = regexp.exec(this.strXml_.substr(pos))) {
                pos = m!.index + 1
                for (let item of m!.result) {
                    let resType = type
                    if (type == EventType.INSTRUCTION && item.substr(0, XmlParseHelper.XML.length) == XmlParseHelper.XML) {
                        resType = EventType.START_DOCUMENT
                        pos = 1
                    } else if (type == EventType.INSTRUCTION) { 
                        pos += item.length
                    }
                    if (this.supportDoctype && (type == EventType.ENTITY_REFERENCE)) {
                        let match = XmlParseHelper.entityRegExp.exec(item)
                        if (match && match.result.length > 3) {
                            let entityName = match.result[1]
                            let entityValue = match.result[3]
                            this.documentEntities.set(entityName, entityValue)
                        }
                    }
                    let node : XmlNode = {
                        type: resType,
                        content: this.removeSpecTags(item),
                        isServiceTagNode: true,
                        isSelfClose: false,
                        pos: pos
                    }
                    this.calcPosition(this.strXml_.substr(0, pos), node)
                    if (resType == EventType.START_DOCUMENT) {
                        if (item != undefined) {
                            node.attributes = this.getAttributes(item, node)
                        }
                        this.startDocumentFound = true
                    }
                    if ((type != EventType.DOCDECL) || this.supportDoctype) {
                        this.nodeMap.set(node!.pos!, node!)
                    }
                }
            }
        }

        private processAllStartTags() {
            let pos: number = 0
            let m : NullishType
            while (m = XmlParseHelper.startTagRegExp.exec(this.strXml_.substr(pos))) {
                pos += m!.index
                let item = m!.result[0]
                pos += item.length
                if (item && !item.match(XmlParseHelper.selfCloseTagRegExp)) {
                    let name = item.substr(1, item.length - 2).split(" ")[0]
                    if (!this.isNodeNameValid(name)) {
                        throw createBusinessError(TypeErrorCodeId,'The node name contains invalid characters');
                    }
                    let node = {
                        pos: pos,
                        type: EventType.START_TAG,
                        name: name,
                        isSelfClose: false
                    } as XmlNode
                    node.attributes = this.getAttributes(item, node)
                    this.processNameSpace(node)
                    this.calcPosition(this.strXml_.substr(0, pos), node!)
                    this.nodeMap.set(node!.pos!, node!)
                }
            }
        }

        private isNodeNameValid(name: string): boolean {
            let m = name.match(XmlParseHelper.nodeNameRegExp)
            if (m && m.result) {
                return m.result.length > 0
            }
            return false
        }

        private processAllEndTags() {
            let pos: number = 0
            let m : NullishType
            while (m = XmlParseHelper.closeTagRegExp.exec(this.strXml_.substr(pos))) {
                pos += m!.index
                let item = m!.result[0]
                pos += item.length
                let name = item.substr(2, item.length - 3)
                if (!this.isNodeNameValid(name)) {
                    throw createBusinessError(TypeErrorCodeId,'The node name contains invalid characters');
                }
                if (item) {
                    let node = {
                        pos: pos,
                        name: name,
                        isSelfClose: false
                    } as XmlNode
                    node!.type = EventType.END_TAG 
                    this.processNameSpace(node)
                    this.calcPosition(this.strXml_.substr(0, pos), node!)
                    this.nodeMap.set(node!.pos!, node!)
                }
            }
        }

        private processAllSelfCloseTags() {
            let m : NullishType
            let pos: number = 0
            while (m = XmlParseHelper.selfCloseTagRegExp.exec(this.strXml_.substr(pos))) {
                pos += m!.index
                let item = m!.result[0]
                if (item) {
                    pos += item.length
                    let name = item.substr(1, item.length - 3).split(" ")[0]
                    if (!this.isNodeNameValid(name)) {
                        throw createBusinessError(TypeErrorCodeId,'The node name contains invalid characters');
                    }
                    let node = {
                        pos: pos,
                        type: EventType.START_TAG,
                        name: name,
                        isSelfClose: true
                    } as XmlNode
                    node.attributes = this.getAttributes(item, node)
                    this.processNameSpace(node) 
                    this.calcPosition(this.strXml_.substr(0, pos), node!)
                    this.nodeMap.set(pos - 1, node)
                    let endNode = {
                        pos: pos,
                        type: EventType.END_TAG,
                        name: name,
                        isSelfClose: true,
                        columnNumber: node.columnNumber,
                        lineNumber: node.lineNumber,
                        prefix: node.prefix,
                        namespace_: node.namespace_
                    } as XmlNode
                    this.nodeMap.set(pos, endNode)
                }
            }
        }

        private processAllTextTags() {
            let m : NullishType
            let pos:number = 0
            while (m = XmlParseHelper.textTagRegExp.exec(this.strXml_.substr(pos))) {
                pos += m!.index
                for (let item of m!.result) {
                    pos += item.length
                    let content = item.substr(1, item.length - 2)
                    let node = {
                        pos: pos,
                        content: this.removeSpecTags(content)
                    } as XmlNode
                    node!.type = (node!.content!.replaceAll(" ", "") == "") ? EventType.WHITESPACE : EventType.TEXT 
                    this.calcPosition(this.strXml_.substr(0, pos - 1), node!)
                    this.nodeMap.set(node!.pos!, node!)
                }
            }
        }

        public parseXml(): void {
            if (this.strXml_ == "") {
                return
            }

            XmlParseHelper.serviceTagRegExpMap.forEach((value: RegExp, key: EventType) => {
                this.processServiceTag(value, key)
            })

            this.processAllStartTags()
            this.processAllSelfCloseTags()
            this.processAllTextTags()
            this.processAllEndTags()

            let keys = Array.from(this.nodeMap.keys()).sort((a, b) => a - b)

            let name = ""
            let depth = 0
            let res = true
            keys.forEach((key: number) => {
                let node = this.nodeMap.get(key)!
                if (node.type == EventType.START_TAG) {
                    name = node!.name!
                    ++depth
                }
                if (node.type == EventType.END_TAG && (depth > 1)) {
                    --depth
                }
                node.depth = depth
                if (res) {
                    node = this.replaceEntities(node)
                    res = this.processNode(node)
                }
            })

            let endNode: XmlNode = {
                name: '',
                type: EventType.END_DOCUMENT,
                depth: 0
            }
            this.calcPosition(this.strXml_, endNode)
            this.processNode(endNode)
        }

        private isNullish(obj: NullishType): boolean {
            return (obj == undefined)
        }

        private getAttributes(str: string, node: XmlNode): Map<string, string> {
            let attributes = new Map<string, string>()
            let keyValueMatch : NullishType
            while (!this.isNullish(keyValueMatch = str.match(XmlParseHelper.attributeRegExp))) {
                if (node.attributes) {
                    attributes = node.attributes!
                }
                attributes!.set(this.replaceXmlSpecChars(keyValueMatch!.result[1] as string),
                    this.replaceXmlSpecChars(this.strip(keyValueMatch!.result[2] as string)))
                str = str.replace(keyValueMatch!.result[0] as string,'')
            }
            return attributes
        }

        private strip(val: string): string {
            return val.replace(XmlParseHelper.stripRegExp, "")
        }

        private nsMap = new Map<string,string>()
        private processNameSpace(node: XmlNode): void {
            if (this.ignoreNameSpace) { 
                return
            }

            if (node.attributes) {
                node.attributes!.forEach ((value: string, key: string) => {
                    if (key! === XmlParseHelper.XMLNS) {
                        this.nsMap.set(XmlParseHelper.UNDESCORE, value)
                    } 
                    if (key.indexOf(XmlParseHelper.XMLNS_PREFIX) === 0) {
                        let prefix = key!.replace(XmlParseHelper.XMLNS_PREFIX,'')
                        this.nsMap.set(prefix, value)
                    }
                })
            }
            if (node.name) {
                let hasPrefix = (node.name!.indexOf(XmlParseHelper.SEMICOLON) > 0)
                if (hasPrefix) {
                    let prefix = node.name!.split(XmlParseHelper.SEMICOLON)[0]
                    if ((prefix != XmlParseHelper.HTTP) && (prefix != XmlParseHelper.HTTPS) && this.nsMap.has(prefix)) {
                        node.name = node.name!.replace(`${prefix}:`,"")
                        node.prefix = prefix
                        node.namespace_ = this.nsMap.get(prefix)
                    }
                } else {
                    if (this.nsMap.get(XmlParseHelper.UNDESCORE)) {
                        node.name = this.nsMap.get(XmlParseHelper.UNDESCORE) + node.name!
                    }
                }
            }
        }

        private onlyStartTag(node: XmlNode): boolean {
            return !(node.type == EventType.END_TAG || node.type == EventType.TEXT || node.type == EventType.WHITESPACE)
        }

        private isStartOrEndDocument(node: XmlNode): boolean {
            return node.type == EventType.START_DOCUMENT || node.type == EventType.END_DOCUMENT
        }

        private processNode(node: XmlNode): boolean {

            if (!this.isNullish(this.tagFunc_) && !(this.isNullish(node.name) && this.isNullish(node.content)) && !this.isStartOrEndDocument(node)) {
                if (!this.tagFunc_!(node.name ? node.name! : "", node.content ? this.replaceXmlSpecChars(node.content!) : "")) {
                    return false
                }
            }
            if (!this.isNullish(this.tokenFunc_)) {
                let info = this.parseToken(node)
                if (!this.tokenFunc_!(node.type != undefined ? node.type! : ((node.content == " ") ? EventType.WHITESPACE : EventType.TEXT), info)) {
                    return false
                }
            }
            if (!this.isNullish(this.attrFunc_) && (!this.isNullish(node.attributes)) && (node.attributes!.size > 0)
            && this.onlyStartTag(node) && !this.isStartOrEndDocument(node)) {
                let res = true
                node.attributes!.forEach((value: string, key: string) => { 
                    if (res && !this.attrFunc_!(key, value)) {
                        res = false
                        return
                    }
                })
                return res
            }
            return true
        }
    }

    const DepthOffset = 1;
    const BufferIndexOffset = 1;
    const IndentationSpaces = 2;

    enum XmlType {
        NONE,
        START_AND_ATTRIBUTES,
        DECLARATION,
        TEXT,
        OTHERS
    }

    export class XmlSerializer {
        private iPos: int = 0;
        private buffer: ArrayBuffer | DataView;
        private encoding: string = 'utf-8';
        private depth: int = 0;
        private type = XmlType.NONE;
        private elementStack = new Array<XmlNode>();
        private multNsp = new Map<int, XmlNode>();
        private isHasDecl: boolean = false;

        /**
         * Constructs an instance of the class with the specified buffer and encoding.
         *
         * @param {ArrayBuffer | DataView} buffer - The buffer containing the data to be processed.
         * @param {string} encoding - The encoding format of the data. Only 'utf-8' is supported.
         * @throws {BusinessError} Throws an error if the encoding is not 'utf-8'.
         */
        constructor(buffer: ArrayBuffer | DataView, encoding: string) {
            if (encoding !== 'utf-8') {
                throw new BusinessError(TypeErrorCodeId, new Error('Parameter error.Just support utf-8'));
            }

            this.buffer = buffer;
            this.increaseStackLength();
        }

        /**
         * Constructs an instance of the class with the specified buffer.
         *
         * @param {ArrayBuffer | DataView} buffer - The buffer containing the data to be processed.
         */
        constructor(buffer: ArrayBuffer | DataView) {
            this.buffer = buffer;
            this.increaseStackLength();
        }

        /**
         * Sets an attribute for the current XML element.
         *
         * @param {string} name - The name of the attribute to set.
         * @param {string} value - The value of the attribute to set.
         * @throws {BusinessError} Throws an error if the method is called in an illegal position.
         */
        public setAttributes(name: string, value: string): void {
            this.checkEmptyParameter(name);
            if (this.type !== XmlType.START_AND_ATTRIBUTES) {
                throw new BusinessError(TypeErrorCodeId, new Error('Illegal position for attribute'));
            }

            let temp = new StringBuilder(` ${name}=\"`);
            this.writeEscaped(temp, value);
            temp.append(`\"`);
            this.type = XmlType.START_AND_ATTRIBUTES;
            this.writeToBuffer(temp, 'SetAttributes');
        }

        /**
         * Handles the start of an XML element.
         *
         * @param {string} name - The name of the XML element being started.
         */
        public startElement(name: string): void {
            this.checkEmptyParameter(name);
            let temp = new StringBuilder();
            if (this.type === XmlType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(temp);
                temp.append('>');
            }
            if (this.type !== XmlType.NONE && this.type !== XmlType.DECLARATION) {
                this.nextItem(temp);
            }

            this.elementStack[this.depth].name = name;
            temp.append('<');
            if (this.elementStack[this.depth].prefix !== '') {
                temp.append(`${this.elementStack[this.depth].prefix}:`);
            } else if (this.depth !== 0) {
                if (this.elementStack[this.depth - DepthOffset].prefix !== '') {
                    this.elementStack[this.depth].prefix = this.elementStack[this.depth - DepthOffset].prefix;
                    temp.append(`${this.elementStack[this.depth].prefix}:`);
                }
            }
            temp.append(`${this.elementStack[this.depth].name}`);
            this.type = XmlType.START_AND_ATTRIBUTES;
            this.depth++;
            this.increaseStackLength();
            this.writeToBuffer(temp, 'StartElement');
        }

        /**
         * Handles the end of an XML element during parsing.
         */
        public endElement(): void {
            let temp = new StringBuilder();
            if (this.type == XmlType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(temp);
                temp.append('/>');
                this.type = XmlType.OTHERS;
                this.depth--;
                this.writeToBuffer(temp, 'EndElement');
                return;
            }
            this.depth--;
            if (this.type !== XmlType.TEXT) {
                this.nextItem(temp);
            }

            temp.append('</');
            if (this.elementStack[this.depth].prefix !== '') {
                temp.append(`${this.elementStack[this.depth].prefix}:`);
            }
            temp.append(`${this.elementStack[this.depth].name}`);
            this.elementStack[this.depth].prefix = '';
            this.elementStack[this.depth].namespace_ = '';
            this.type = XmlType.OTHERS;
            temp.append('>');
            this.writeToBuffer(temp, 'EndElement');
        }

        /**
         * Sets the namespace for the current XML element.
         *
         * @param {string} prefix - The namespace prefix to be set.
         * @param {string} namespace - The namespace URI associated with the prefix.
         */
        public setNamespace(prefix: string, namespace: string): void {
            this.checkEmptyParameter(prefix);
            if (namespace.length === 0) {
                throw new BusinessError(TypeErrorCodeId,
                    new Error(`Parameter error. The type of ${namespace} must be string`));
            }
            let temp = new StringBuilder();
            if (this.type === XmlType.START_AND_ATTRIBUTES) {
                this.spliceNamespace(temp);
                temp.append('>');
            }

            this.elementStack[this.depth].prefix = prefix;
            this.elementStack[this.depth].namespace_ = namespace;
            let node: XmlNode = {
                prefix: this.elementStack[this.depth]!.prefix,
                namespace_: this.elementStack[this.depth]!.namespace_
            }
            this.multNsp.set(this.depth, node);
            this.type = XmlType.OTHERS;
            this.writeToBuffer(temp, 'SetNamespace');
        }

        /**
         * Increases the length of the element stack by creating a new `XmlNode` object with default empty values for
         * `name`, `prefix`, and `namespace_`, and pushing it onto the `elementStack`.
         */
        private increaseStackLength(): void {
            let node: XmlNode = {
                name: '',
                prefix: '',
                namespace_: ''
            }
            this.elementStack.push(node);
        }

        /**
         * Escapes special characters in the given string and appends the result to the provided StringBuilder.
         *
         * @param {StringBuilder} stringBuilder - The StringBuilder to which the escaped string will be appended.
         * @param {string} s - The input string to be escaped.
         */
        private writeEscaped(stringBuilder: StringBuilder, s: string): void {
            s = s.replaceAll('&', '&amp;');
            s = s.replaceAll('\'', '&apos;');
            s = s.replaceAll('\"', '&quot;');
            s = s.replaceAll('>', '&gt;');
            s = s.replaceAll('<', '&lt;');
            stringBuilder.append(s);
        }

        /**
         * Writes the content of the `stringBuilder` array to the `buffer` starting at the current position (`iPos`).
         * If the buffer does not have enough space to accommodate the data, an error is thrown.
         *
         * @param {StringBuilder} stringBuilder - The StringBuilder will be write.
         * @param {string} name - The name used in the error message if the operation fails.
         * @throws {BusinessError} Throws an error if the buffer does not have enough space to write the data.
         */
        private writeToBuffer(stringBuilder: StringBuilder, name: string): void {
            let tempString = stringBuilder.toString();
            let iLenTemp = tempString.length;
            if (this.buffer.byteLength > this.iPos + iLenTemp - BufferIndexOffset) {
                for (let i = 0; i < iLenTemp; i++) {
                    if (this.buffer instanceof ArrayBuffer) {
                        (this.buffer as ArrayBuffer).set((i + this.iPos) as int, tempString[i] as byte);
                    } else {
                        ((this.buffer as DataView).buffer as ArrayBuffer).set((i + this.iPos) as int,
                            tempString[i] as byte);
                    }
                }
                this.iPos += iLenTemp;
            } else {
                throw new BusinessError(TypeErrorCodeId, new Error(`XmlSerializer:: ${name} memcpy_s failed`));
            }
        }

        /**
         * Handles the namespace processing for the current element in the stack.
         *
         * @param {StringBuilder} stringBuilder - The `StringBuilder` instance used to append
         * namespace declarations to the XML output.
         */
        private spliceNamespace(stringBuilder: StringBuilder): void {
            if (this.depth === 0) {
                return;
            }

            this.elementStack[this.depth].prefix = this.elementStack[this.depth - DepthOffset].prefix;
            this.elementStack[this.depth].namespace_ = this.elementStack[this.depth - DepthOffset].namespace_;

            let prevDepthInfo = this.multNsp.get(this.depth - DepthOffset);
            if (prevDepthInfo === undefined) {
                return;
            }

            if (this.type === XmlType.START_AND_ATTRIBUTES) {
                let prefix = prevDepthInfo.prefix;
                let namespace = prevDepthInfo.namespace_;
                stringBuilder.append(` xmlns:${prefix}=\"${namespace}\"`);
                this.multNsp.get(this.depth - DepthOffset)!.prefix = '';
                this.multNsp.get(this.depth - DepthOffset)!.namespace_ = '';

            }
        }

        /**
         * Appends a newline character followed by indentation spaces to the `out` property.
         *
         * @param {StringBuilder} stringBuilder - The `StringBuilder` instance used to append spaces to the XML output.
         */
        private nextItem(stringBuilder: StringBuilder): void {
            stringBuilder.append('\r\n');
            stringBuilder.append(' '.repeat(IndentationSpaces * this.depth));
        }

        /**
         * Checks if the provided parameter is an empty string.
         * Throws a `BusinessError` if the parameter is empty.
         *
         * @param {string} para - The parameter to be checked.
         */
        private checkEmptyParameter(para: string): void {
            if (para.length === 0) {
                throw new BusinessError(TypeErrorCodeId, new Error('Parameter error. Parameter cannot be empty'));
            }
        }
    }
}
