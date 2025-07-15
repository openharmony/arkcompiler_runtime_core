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

package std.core;

export class Formatter {
    internal static objectToString(obj: Object, full: boolean = false): string {
        return (new Formatter).stringifyObject(obj, '\n', 1, full)
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

        if (Formatter.isNullObject(subObj)) {
            return Formatter.nullObjectDescription(subObjName) + ',' + delimiter
        }

        let strB = new StringBuilder
        subObj = __narrowAny(subObj)

        if (Formatter.isArray(subObj!)) {
            strB.append(subObjName + ':' + delimiter + Formatter.arrayToString(delimiter + '  ', subObj! as Array<Object>, depth, full))
        } else if (Formatter.isObject(subObj!)) {
            strB.append(this.subObjectDescription(subObj!, subObjName, this.isCircular(subObj!), delimiter, depth))
        } else if (Formatter.isFunction(subObj!)) {
            strB.append(Formatter.functionDescription(subObj!, delimiter, true, full))
        } else {
            strB.append(Formatter.simpleObjectDescription(subObj!, subObjName, true))
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

        return this.objects.has(__narrowAny<object>(obj))
    }

    private static nullObjectDescription(objName: string = ''): string {
        return ((objName != '') ? objName + ': ' : '') + null
    }

    private static simpleObjectDescription(obj: Object, objName: string = '', isSub: boolean = false): string {
        let quotesStr = Formatter.isString(obj) ? '\'' : ''
        return (isSub ? objName + ': ' : '') + quotesStr + obj + quotesStr
    }

    private stringifyObject(obj: NullishType, delimiter: string = '\n', depth: number = 1, full: boolean = false): string {

        if (Formatter.isNullObject(obj)) {
            return Formatter.nullObjectDescription()
        }

        obj = __narrowAny(obj)
        this.objects.add(obj!)
        let strB = new StringBuilder
        if (Formatter.isArray(obj!)) {
            strB.append(Formatter.arrayToString(delimiter, obj! as Array<Object>, depth, full))
        } else if (Formatter.isFunction(obj!)) {
            strB.append(Formatter.functionDescription(obj!, delimiter, false, full))
            if (!full) {
                strB = (new StringBuilder).append('{ ').append(strB.toString()).append(' }')
            }
        } else if (Formatter.isObject(obj!)) {
            strB.append(this.objectDescription(obj!, delimiter, depth, full))
        } else {
            strB.append(Formatter.simpleObjectDescription(obj!, ''))
        }
        return strB.toString()
    }

    private static arrayToString(delimiter: string, arr: Array<Object>, depth: number, full: boolean = false): string {
        return ('[ ' + Formatter.arrayToStringImpl(delimiter, arr, depth, full) + (full ? ('[length]: ' + arr.length) : '') + ' ]')
    }

    private static functionDescription(obj: Object, delimiter: string = '\n', isSub: boolean = false, full: boolean = false, inArray: boolean = false): string {
        let strB = new StringBuilder
        let functionName = Formatter.getName(obj)
        strB.append((isSub && functionName !== '') ? (functionName + ':' + delimiter) : '');

        let baseFunctionStr = '[Function: ' + functionName + ']'

        if (full) {
            let end = delimiter + (inArray || isSub) ? '  ' : '';
            let length = Formatter.getLength(obj)
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
            if (!Formatter.isNullObject(item) && (Formatter.isFunction(item) || Formatter.isObject(item) && depth <= 2)) {
                arrayDelimiter += delimiter
                break
            }
        }

        let strB = new StringBuilder
        for (let item of arr) {
            if (Formatter.isObject(item)) {
                strB.append((new Formatter).stringifyObject(item, delimiter + '  ', depth + 1, full))
            } else if (Formatter.isFunction(item)) {
                strB.append(Formatter.functionDescription(item, delimiter, false, full, true))
            } else {
                strB.append(Formatter.simpleObjectDescription(item, full ? Formatter.getName(item) : ''))
            }
            strB.append(arrayDelimiter)
        }

        return strB.toString().slice(0, -arrayDelimiter.length)
    }

    private static getName(obj: Object): string {
        let subObj = Formatter.getValue("name", obj)
        return subObj ? subObj! as string : 'null'
    }

    private static getLength(obj: Object): int {
        let subObj = Formatter.getValue("length", obj)
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
    private static extractContents(html: string): string {
        return html
            .replace(new RegExp("(\n|\r|\t)","gm"), '') // remove linebreaks
            .replace(new RegExp("<(style|script|link|noscript).*?>.*?<\/(style|script|link|noscript)>","g"), '') // remove css, js blocks
            .replace(new RegExp("<!--.*?-->","g"), '') // remove comments
            .replace(new RegExp("<.*?>","g"), '') // remove tags
            .replace(new RegExp("[\s\!\@\#\$\%\^\&\*\(\)\_\+\-\=\{\}\[\]\:\"\;\'\<\>\?\,\.\/\|\\\`\~]+","g"), '') // remove Chars
            .trim()
    }

    private static processArg(flag: string, arg: Object): string {
        // %c: CSS. This specifier is ignored and will skip any CSS passed in.
        if (flag == "c" && (arg instanceof String)) {
            return Formatter.extractContents(arg as string)
        } else if (flag == "s" && !(arg instanceof BigInt)) {
            return arg as string
        } else if (flag == "d") {
            if (arg instanceof BigInt) {
                throw new Error("invalid flag " + flag + " for arg " + arg as string)
            }
            return "" + new Number(arg.toString())
        } else if (flag == "i") {
            if (arg instanceof BigInt) {
                throw new Error("invalid flag " + flag + " for arg " + arg as string)
            }
            return "" + parseInt(arg as string, 10)
        }
        else if (flag == "f") {
            return "" + parseFloat(arg as string)
        } else if (flag == "j") {
            let res = ""
            try {
                res = JSON.stringify(arg)
            }
            catch(error) {
                if (typeof arg === "object") {
                    return Formatter.objectToString(arg)
                } else {
                    throw new Error("error parse json")
                }
            }
            return res
        } else if (flag == "o") {
            // Object. A string representation of an object
            // Similar to util.inspect() with options { showHidden: true, showProxy: true }
            return Formatter.objectToString(arg, true)
        } else if (flag == "O") {
            // Object. A string representation of an object
            // Similar to util.inspect() without options
            return Formatter.objectToString(arg)
        }

        return "%" + flag
    }

    private static joinLastArgs(index: int, args: Array<NullishType>): string {
        let sb = new StringBuilder
        for (let i = index; i < args.length; ++i) {
            sb.append(" ")
            // behavior for null and undefined expected to be the same as for empty string
            sb.append(args[i] != undefined && args[i] != null ? __narrowAny(args[i])!.toString() : "")
        }
        return sb.toString()
    }

    // NOTE(ivan-tyulyandin): some modifiers work wrongly, see #25046
    public format(s: string, args: Array<NullishType>): string {
        let preI: int = 0
        let i: int = 0
        let argCounter = 0
        const flags: string = "cdfijoOs"
        let res = new StringBuilder
        let ss = ""
        if (args.length == 0) {
            return s.replaceAll("%%", "%")
        }

        while((i = ((ss = s.substring(preI, s.length)).search("%")).toInt()) != -1 && (preI + i < s.length - 1) && (argCounter < args.length)) {
            res.append(s.substring(preI, preI + i))
            let flagIndex = preI + i + 1
            //NOTE(DZ): temporary solution until primitives remove patch will be merged
            let ch = new Char(s[flagIndex]);
            let flag: string = ch.toString();
            if (flags.includes(flag)) {
                let d = args[argCounter]
                if (d != undefined && d != null) {
                    res.append(Formatter.processArg(flag, __narrowAny<object>(d)))
                } else {
                    // behavior for null and undefined expected to be the same as for empty string
                    res.append(Formatter.processArg(flag, ""))
                }

                ++argCounter
            } else if (flag == "%") {
                res.append("%")
            } else {
                res.append("%" + flag)
            }
            preI += i + 2
        }
        if (preI < s.length) {
            res.append(s.substring(preI,s.length))
        }
        let strArgsList = Formatter.joinLastArgs(argCounter, args)
        let resStr = res.toString()
        return resStr.length == 0 ? res + strArgsList.slice(1) : res + strArgsList
    }

}
