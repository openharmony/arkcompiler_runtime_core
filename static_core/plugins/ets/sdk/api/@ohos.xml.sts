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

package api

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

    class ParseInfoImpl implements ParseInfo {
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
        public parse(option: ParseOptions) : void {
            this.helper.supportDoctype = (option.supportDoctype != undefined) ? option.supportDoctype! : false
            this.helper.ignoreNameSpace = (option.ignoreNameSpace != undefined) ? option.ignoreNameSpace! : false
            this.helper.tagFunc_ = option.tagValueCallbackFunction
            this.helper.attrFunc_ = option.attributeValueCallbackFunction
            this.helper.tokenFunc_ = option.tokenValueCallbackFunction
            this.helper.parse()
        }
    }
}
