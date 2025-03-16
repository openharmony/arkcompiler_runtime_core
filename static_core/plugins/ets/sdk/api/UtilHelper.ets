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

export namespace util {

class UtilHelper {

    static {
        loadLibrary("ets_sdk_native")
    }

    internal static native generateRandomUUID(entropyCache: boolean): string

    internal static objectToString(obj: Object, full: boolean = false): string {
        return (new UtilHelper).stringifyObject(obj, '\n', 1, full)
    }

    private static isNullObject(obj: NullishType): boolean {
        return (obj == undefined) || (obj == null)
    }

    private static isFunction(obj: Object): boolean {
        return (typeof obj === 'function')
    }

    private static isObject(obj: Object): boolean {
        return (typeof obj === 'object')
    }

    private static isString(obj: Object): boolean {
        return (typeof obj === 'string')
    }

    private static isArray(obj: Object): boolean {
        return obj instanceof Array
    }

    private stringifySubObject(delimiter: string, subObj: NullishType, subObjName: string, depth: number, full: boolean = false): string {

        if (UtilHelper.isNullObject(subObj)) {
            return UtilHelper.nullObjectDescription(subObjName) + ',' + delimiter
        }

        let strB = new StringBuilder

        if (UtilHelper.isArray(subObj!)) {
            strB.append(subObjName + ':' + delimiter + UtilHelper.arrayToString(delimiter + '  ', subObj! as Array<Object>, depth, full))
        } else if (UtilHelper.isObject(subObj!)) {
            strB.append(this.subObjectDescription(subObj!, subObjName, this.isCircular(subObj!), delimiter, depth))
        } else if (UtilHelper.isFunction(subObj!)) {
            strB.append(UtilHelper.functionDescription(subObj!, delimiter, true, full))
        } else {
            strB.append(UtilHelper.simpleObjectDescription(subObj!, subObjName, true))
        }
        this.objects.add(subObj!)

        strB.append(',' + delimiter)
        return strB.toString()
    }

    private objectDescription(obj: Object, delimiter: string = '\n', depth: number = 1, full: boolean = false): string {

        let strB = new StringBuilder('{ ')
        let isComplexObject: boolean = false

        for (const objEntry of Object.entries(obj)) {
            if (!objEntry) {
                continue
            }
            isComplexObject = true
            strB.append(this.stringifySubObject(delimiter, objEntry[1], objEntry[0], depth, full))
        }

        if (!isComplexObject) {
            return JSON.stringify(obj)
        }

        return strB.toString().slice(0,- delimiter.length - 1) + ' }'
    }

    private subObjectDescription(obj: Object, objName: string, isCircular: boolean, delimiter: string, depth: number): string {
        if (isCircular) {
            return objName + ': [Circular]' + delimiter
        }
        return objName + ':' + delimiter + this.stringifyObject(obj, delimiter + '  ', depth + 1, true)
    }

    private objects = new Set<Object>

    private isCircular(obj: NullishType): boolean {
        if (obj == null) {
            return false
        }

        return this.objects.has(obj)
    }

    private static nullObjectDescription(objName: string = ''): string {
        return ((objName != '') ? objName + ': ' : '') + null
    }

    private static simpleObjectDescription(obj: Object, objName: string = '', isSub: boolean = false): string {
        let quotesStr = UtilHelper.isString(obj) ? '\'' : ''
        return (isSub ? objName + ': ' : '') + quotesStr + obj + quotesStr
    }

    private stringifyObject(obj: NullishType, delimiter: string = '\n', depth: number = 1, full: boolean = false): string {

        if (UtilHelper.isNullObject(obj)) {
            return UtilHelper.nullObjectDescription()
        }

        this.objects.add(obj!)
        let strB = new StringBuilder
        if (UtilHelper.isArray(obj!)) {
            strB.append(UtilHelper.arrayToString(delimiter, obj! as Array<Object>, depth, full))
        } else if (UtilHelper.isFunction(obj!)) {
            strB.append(UtilHelper.functionDescription(obj!, delimiter, false, full))
            if (!full) {
                strB = (new StringBuilder).append('{ ').append(strB.toString()).append(' }')
            }
        } else if (UtilHelper.isObject(obj!)) {
            strB.append(this.objectDescription(obj!, delimiter, depth, full))
        } else {
            strB.append(UtilHelper.simpleObjectDescription(obj!, ''))
        }
        return strB.toString()
    }

    private static arrayToString(delimiter: string, arr: Array<Object>, depth: number, full: boolean = false): string {
        return ('[ ' + UtilHelper.arrayToStringImpl(delimiter, arr, depth, full) + (full ? ('[length]: ' + arr.length) : '') + ' ]')
    }

    private static functionDescription(obj: Object, delimiter: string = '\n', isSub: boolean = false, full: boolean = false, inArray: boolean = false): string {
        let strB = new StringBuilder
        let functionName = UtilHelper.getName(obj)
        strB.append((isSub && functionName !== '') ? (functionName + ':' + delimiter) : '');

        let baseFunctionStr = '[Function: ' + functionName + ']'

        if (full) {
            let end = delimiter + (inArray || isSub) ? '  ' : '';
            let length = UtilHelper.getLength(obj)
            strB.append('{ ' + baseFunctionStr + end +
                '[length]: ' + (length ? length : 0) + ',' + end +
                '[name] :\'' + functionName + '\',' + end +
                '[prototype]: ' + functionName + ' { [constructor]: [Circular] } }')
        } else {
            strB.append(baseFunctionStr)
        }

        return strB.toString()
    }

    private static arrayToStringImpl(delimiter: string, arr: Array<Object>, depth: number, full: boolean = false): string {
        if (!arr.length) {
            return ''
        }

        let arrayDelimiter: string = ', '

        for (let item of arr) {
            if (!UtilHelper.isNullObject(item) && (UtilHelper.isFunction(item) || UtilHelper.isObject(item) && depth <= 2)) {
                arrayDelimiter += delimiter
                break
            }
        }

        let strB = new StringBuilder
        for (let item of arr) {
            if (UtilHelper.isObject(item)) {
                strB.append((new UtilHelper).stringifyObject(item, delimiter + '  ', depth + 1, full))
            } else if (UtilHelper.isFunction(item)) {
                strB.append(UtilHelper.functionDescription(item, delimiter, false, full, true))
            } else {
                strB.append(UtilHelper.simpleObjectDescription(item, full ? UtilHelper.getName(item) : ''))
            }
            strB.append(arrayDelimiter)
        }

        return strB.toString().slice(0, -arrayDelimiter.length)
    }

    private static getName(obj: Object): string {
        let subObj = UtilHelper.getValue("name", obj)
        return subObj ? subObj! as string : 'null'
    }

    private static getLength(obj: Object): int {
        let subObj = UtilHelper.getValue("length", obj)
        return subObj ? subObj! as int : 0
    }

    private static getValue(key: string, obj: Object): NullishType {
        if (!obj) {
            return null
        }
        for (const objEntry of Object.entries(obj)) {
            if (!objEntry) {
                continue
            }
            if ((!objEntry[0]) && (objEntry[0] == key)) {
                return objEntry[1]
            }
        }
        return null
    }

    /**
      * Remove html tag, css, js of html string to extract the contents
      * @param {String} html
      * @return {String} contents of the html
      */
    static extractContents(html: string): string {
        return html
            .replace(new RegExp("(\n|\r|\t)","gm"), '') // remove linebreaks
            .replace(new RegExp("<(style|script|link|noscript).*?>.*?<\/(style|script|link|noscript)>","g"), '') // remove css, js blocks
            .replace(new RegExp("<!--.*?-->","g"), '') // remove comments
            .replace(new RegExp("<.*?>","g"), '') // remove tags
            .replace(new RegExp("[\s\!\@\#\$\%\^\&\*\(\)\_\+\-\=\{\}\[\]\:\"\;\'\<\>\?\,\.\/\|\\\`\~]+","g"), '') // remove Chars
            .trim()
    }

    static processArg(flag: string, arg: Object): string {
        // %c: CSS. This specifier is ignored and will skip any CSS passed in.
        if (flag == "c" && (arg instanceof String)) {
            return UtilHelper.extractContents(arg as string)
        } else if (flag == "s" && !(arg instanceof BigInt)) {
            if (arg instanceof BigInt || arg instanceof Char) {
                throw new Error("invalid flag " + flag + " for arg " + arg as string)
            }
            return arg as string
        } else if (flag == "d") {
            if (arg instanceof BigInt || arg instanceof Char) {
                throw new Error("invalid flag " + flag + " for arg " + arg as string)
            }
            return "" + new Number(arg.toString())
        } else if (flag == "i") {
            if (arg instanceof BigInt || arg instanceof Char) {
                throw new Error("invalid flag " + flag + " for arg " + arg as string)
            }
            return "" + parseInt(arg as string, 10)
        }
        else if (flag == "f") {
            if (arg instanceof Char) {
                throw new Error("invalid flag " + flag + " for arg " + arg as string)
            }
            return "" + parseFloat(arg as string)
        } else if (flag == "j") {
            let res = ""
            try {
                res = JSON.stringify(arg)
            }
            catch(error) {
                if (typeof arg === "object") {
                    return UtilHelper.objectToString(arg)
                } else {
                    throw new Error("error parse json")
                }
            }

        return res
        } else if (flag == "o") {
            // Object. A string representation of an object
            // Similar to util.inspect() with options { showHidden: true, showProxy: true }
            return UtilHelper.objectToString(arg, true)
        } else if (flag == "O") {
            // Object. A string representation of an object
            // Similar to util.inspect() without options
            return UtilHelper.objectToString(arg)
        }

        return "%" + flag
    }

    static joinLastArgs(index: int, ...args: Object[]): string {
        let sb = new StringBuilder
        for (let i = index; i < args.length; ++i) {
            sb.append(" ")
            sb.append(args[i])
        }
        let res = sb.toString()
        return res
    }

}  // class UtilHelper
} // namespace util
