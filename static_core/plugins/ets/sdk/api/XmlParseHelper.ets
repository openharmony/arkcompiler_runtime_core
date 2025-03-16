/*
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

package api

export namespace xml {

    type Attributes = Map<string, string>

    interface XmlNode {
        name?: string
        attributes?: Attributes
        content?: string
        children?: Array<XmlNode>
        lineNumber?: number
        columnNumber?: number
        depth?: number
        prefix?: string
        namespace_?: string
        type?: EventType
        isSelfClose?: boolean
    }

    class ParseAllResult {
        children: Array<XmlNode> = {}
        strLeft: string
        lineNumber: number = 0
        columnNumber: number = 0
        depth: number = 0
    }

    class ParseTagResult {
        type: 'selfClose' | 'normal' = 'normal'
        name: string = ''
        content: NodeParseResult | string = ''
    }

    class NodeParseResult {
        outer: string = ""
        attrs: string = ""
        inner: string = ""
        pos: number = 0
    }

    export class XmlParseHelper {

        public supportDoctype: boolean = false
        public ignoreNameSpace: boolean = false
        public tagFunc_?: (name: string, value: string) => boolean
        public attrFunc_?: (name: string, value: string) => boolean
        public tokenFunc_?: (eventType: EventType, value: ParseInfo) => boolean

        private strXml_ : string = " "

        constructor(str: string) {
            this.strXml_ = str
        }

        private static readonly const XML = "<?xml"
        private static readonly const SEMICOLON = ':'
        private static readonly const HTTP = 'http'
        private static readonly const HTTPS = 'https'
        private static readonly const UNDESCORE = '_'
        private static readonly const XMLNS = 'xmlns'
        private static readonly const XMLNS_PREFIX = 'xmlns:'
        private static readonly const lineEndRegExp = new RegExp("\n", "m")
        private static readonly const selfCloseTagRegExp = new RegExp("<([\\w:]+)([^<^>])*?/>", "m")
        private static readonly const normalTagRegExp = new RegExp("<([\\w:]+)[\\s\\S]*?>[\\s\\S]*?<\*?>", "m")
        private static readonly const startTagRegExp = new RegExp("<([\\w-:.]+)\\s*","m")
        private static readonly const selfCloseTagParamRegExp = (tagName: string) => { return new RegExp(`<${tagName}[^<^>]*?\/>`,'m') }
        private static readonly const normalTagParamRegExp = (tagName: string) => { return new RegExp(`<${tagName}([\\s\\S]*?)>([\\s\\S]*?)<\/${tagName}>`,'gm') }
        private static readonly const attributeRegExp = new RegExp("([\\w:-]+)\\s*=\\s*(\"[^\"]*\"|'[^']*'|\\w+)\\s*", "m")
        private static readonly const stripRegExp = new RegExp("^['\"]|['\"]$","g")

        private static readonly const serviceTagRegExpMap = new Map<EventType, RegExp>([
            [EventType.INSTRUCTION, new RegExp("<\\?[^>]+?>","g")],
            [EventType.COMMENT, new RegExp("<!--[\\s\\S]*?-->","g")],
            [EventType.ENTITY_REFERENCE, new RegExp("<!ENTITY[\\s\\S]*?>","g")],
            [EventType.DOCDECL, new RegExp("<!DOCTYPE[\\s\\S]*?\[[\\s\\S]*?\]>","g")],
            [EventType.CDSECT, new RegExp("<!\\[CDATA\\[[\\s\\S]*?\]\]>","gm")]
        ])

        public calcPosition(str: string, node: XmlNode) {
            let lineEndMatch: NullishType
            let pos: number = 0
            let end = str.length > 1 ? str.length - 1 : 0
            let lineChanged: boolean = false
            node.lineNumber = 0
            node.columnNumber = 0
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
            info.depth = node.depth ? node.depth! : 0
            info.columnNumber = node.columnNumber ? node.columnNumber! : 0
            info.lineNumber = node.lineNumber ? node.lineNumber! : 0
            info.attributeCount = node.attributes ? node.attributes!.size : 0
            info.emptyElementTag = node.isSelfClose ? (node.isSelfClose!) : false
            info.whitespace = node.content ? (node.content! == " " || node.content! == "") : true
            info.namespace_ = node.namespace_ ? node.namespace_! : ""
            info.name = node.name ? node.name! : ""
            info.prefix = node.prefix ? node.prefix! : ""
            info.text = node.content ? node.content! : ""

            return info
        }

        private processServiceTag(regexp: RegExp, type: EventType) {
            let m : NullishType
            let pos: number = 0
            while (m = regexp.exec(this.strXml_.substr(pos))) {
                pos = m!.index + 1
                for (let item of m!.result) {
                    let resType = type
                    if (type == EventType.INSTRUCTION && item.substr(0, XmlParseHelper.XML.length) == XmlParseHelper.XML) {
                        resType = EventType.START_DOCUMENT
                    }
                    let node : XmlNode = {
                        type: resType,
                        content: item
                    }
                    let index = m!.index
                    this.calcPosition(this.strXml_.substr(0, index), node)
                    if (resType == EventType.START_DOCUMENT) {
                        if (item != undefined) {
                            node.attributes = this.getAttributes(item, node)
                        }
                    }
                    if ((type != EventType.DOCDECL) || this.supportDoctype) {
                        this.processNode(node)
                    }
                }
            }
        }

        public parse(): void {

            XmlParseHelper.serviceTagRegExpMap.forEach((value: RegExp, key: EventType) => {
                this.processServiceTag(value, key)
            })

            let xml = this.strXml_
            let parsed = this.parseNodeRecursive(xml)
            if (parsed) {
                if ((parsed.children.length === 1) && !(parsed.children[0] instanceof String)) {

                    let endNode: XmlNode = {
                        name: '',
                        type: EventType.END_DOCUMENT,
                        depth: 0
                    }
                    this.calcPosition(this.strXml_, endNode)
                    this.processNode(endNode)

                } else {
                    //NOTE(ekaterinazaytseva): should be replaced with BusinessError
                    throw new Error('Failed to parse XML')
                }
            }
        }

        private isNullish(obj: NullishType): boolean {
            return (obj == undefined)
        }

        private parseNodeRecursive(str: string, depth: number = 0, pos: number = 0): ParseAllResult | undefined {
            let all = new Array<XmlNode>
            
            let firstTag: ParseTagResult | undefined
            while (!this.isNullish(firstTag = this.getStartTag(str, pos))) {
                let targetStr : string = ''
                let node : XmlNode | undefined = undefined
                if (firstTag!.type === 'selfClose') {
                    targetStr = firstTag!.content as string
                    pos += this.strXml_.substr(pos).search(targetStr)
                    node = this.parseNode(targetStr, firstTag!.name, true, depth + 1, "", pos)
                } else if (firstTag!.type === 'normal') {
                    let parseRes = (firstTag!.content as NodeParseResult)
                    targetStr = parseRes.outer
                    pos += this.strXml_.substr(pos).search(targetStr)
                    node = this.parseNode(firstTag!.content, firstTag!.name, false, depth + 1, str, pos)
                }
                if (!this.isNullish(node)) {
                    all.push(node!)
                }
                str = str.replace(targetStr, '')
            }
            if (all.length === 0) {
                return undefined
            } else {
                return {
                    children: all,
                    strLeft: str
                } as ParseAllResult
            }
        }

        private parseNode(target: string | NodeParseResult, tagName: string ,isSelfClose: boolean, depth: number = 0, originStr: string = "", pos: number = 0): XmlNode | undefined {
            let node: XmlNode = {
                name: tagName,
                depth: depth + 1
            }
            if (isSelfClose) {
                node = this.parseSelfCloseTag(target as string, node)
                node.isSelfClose = true
                node.type = EventType.START_TAG
                this.calcPosition(this.strXml_.substr(0, pos), node)
                this.processNode(node)
            } else {
                let t: NodeParseResult = target as NodeParseResult
                node = this.parseNormalTag(t, node)

                let startNode = {type: EventType.START_TAG, name: node.name, depth: depth} as XmlNode
                startNode.content = (target as NodeParseResult).inner
                startNode.attributes = node.attributes
                this.calcPosition(this.strXml_.substr(0, pos), startNode)
                this.processNode(startNode)

                node.content = startNode.content
                node.type = (node.content == " " || node.content == "") ? EventType.WHITESPACE : EventType.TEXT 
                // process text
                const startTagText = new StringBuilder().append("<").append(node.name!).append(t.attrs!).append(">").toString()
                this.calcPosition(this.strXml_.substr(0, pos + startTagText.length), node)
                this.processNode(node)

                let endTagNode = {
                    name: "/" + node.name, 
                    namespace_: node.namespace_,
                    prefix: node.prefix, 
                    content: "",
                    type: EventType.END_TAG, 
                    depth: depth
                } as XmlNode
                this.calcPosition(this.strXml_.substr(0, pos + startTagText.length + node.content!.length), endTagNode)
                this.processNode(endTagNode)
            }
            return node
        }

        private parseSelfCloseTag(str: string, node: XmlNode) : XmlNode {
            if (str != undefined) {
                node.attributes = this.getAttributes(str, node)
                this.processNameSpace(node)
                return node
            }
            return {}
        }

        private parseNormalTag(parseResult: NodeParseResult, node: XmlNode): XmlNode {
            if (parseResult.attrs != undefined) {
                node.attributes = this.getAttributes(parseResult.attrs, node)
            }
            let str = parseResult.inner
            this.processNameSpace(node)

            let selfCloseTagMatch = str.match(XmlParseHelper.selfCloseTagRegExp)
            let normalTagMatch = str.match(XmlParseHelper.normalTagRegExp)

            if (!this.isNullish(selfCloseTagMatch) || !this.isNullish(normalTagMatch)) {
                let res = this.parseNodeRecursive(str, node.depth!, parseResult.pos)
                if (res) {
                    if (res.children && res.children.length > 0) {
                        node.children = res.children
                    }
                    if (res.strLeft !== '') {
                        node.content = res.strLeft
                    }
                }
            } else {
                if (str != '') {
                    node.content = str 
                }
            }
            
            return node
        }

        private getStartTag(str: string, pos: number): ParseTagResult | undefined {
            let startTagMatch = str.match(XmlParseHelper.startTagRegExp)
            if (!startTagMatch) {
                return undefined
            }
            let tagName = startTagMatch.result[1]
            if (tagName == undefined) {
                return undefined
            }
            pos += startTagMatch.index
            let selfCloseStr = this.getSelfCloseNode(str, tagName)
            if (selfCloseStr) {
                return {
                    type: 'selfClose',
                    name: tagName,
                    content: selfCloseStr
                }  as ParseTagResult
            }
            let normalContent = this.getNormalNode(str, tagName, pos)
            if (normalContent) {
                return {
                    type: 'normal',
                    name: tagName,
                    content: normalContent
                } as ParseTagResult
            }
            return undefined
        }

        private getSelfCloseNode(str: string, tagName: string): string | undefined {
            let selfCloseTagMatch = str.match(XmlParseHelper.selfCloseTagParamRegExp(tagName))
            if (selfCloseTagMatch) {
                return selfCloseTagMatch.result[0]
            }
            return undefined
        }

        private getNormalNode(str: string, tagName: string, pos: number): NodeParseResult | undefined {
            let normalTagMatch = XmlParseHelper.normalTagParamRegExp(tagName).exec(str)
            if (normalTagMatch) {
                pos += normalTagMatch.index + pos
                return {
                    outer: normalTagMatch.result[0],
                    attrs: normalTagMatch.result[1],
                    inner: normalTagMatch.result[2],
                    pos: pos
                } as NodeParseResult
            }
            return undefined
        }

        private getAttributes(str: string, node: XmlNode): Map<string, string> {
            let attributes = new Map<string, string>()
            let keyValueMatch : NullishType
            while (!this.isNullish(keyValueMatch = str.match(XmlParseHelper.attributeRegExp))) {
                if (node.attributes) {
                    attributes = node.attributes!
                }
                attributes!.set(keyValueMatch!.result[1] as string, this.strip(keyValueMatch!.result[2] as string))
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

        private onlyStartTag(node: XmlNode) {
            return !(node.type == EventType.END_TAG || node.type == EventType.TEXT || node.type == EventType.WHITESPACE)
        }

        private processNode(node: XmlNode) {
            if (!this.isNullish(this.tagFunc_) && !(this.isNullish(node.name) && this.isNullish(node.content))
            && this.onlyStartTag(node)) {
                if (!this.tagFunc_!(node.name ? node.name! : "", node.content ? node.content!: "")) {
                    return
                }
            }
            if (!this.isNullish(this.tokenFunc_)) {
                if (!this.tokenFunc_!(node.type ? node.type! : ((node.content == " ") ? EventType.WHITESPACE : EventType.TEXT), //this.info)) {
                this.parseToken(node))) {
                    return
                }
            }
            if (!this.isNullish(this.attrFunc_) && (!this.isNullish(node.attributes)) && (node.attributes!.size > 0)
            && this.onlyStartTag(node)) {
                let res = true
                node.attributes!.forEach((key: string, value: string) => { 
                    if (res && !this.attrFunc_!(key, value)) {
                        res = false
                        return
                    }
                })
            }
        }
    }
}
