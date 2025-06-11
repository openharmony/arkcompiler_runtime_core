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

import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: number = 401;
const SyntaxErrorCodeId: number = 10200002;

function createBusinessError(code: number, message:string){
    let err = new BusinessError();
    err.code = code;
    err.name='BusinessError';
    err.message = message;
    return err;
}

export namespace url {

    class Helper {
        static ASCII_32: number = 32;
        static ASCII_127: number = 127;
        static specialCharForBit: Array<number> = new Array<number>(128);
        static head: Map<string, number> = new Map<string, number>();
        static doubleSegment = new Array<string>("..", ".%2e", ".%2E", "%2e.", "%2E.",
            "%2e%2e", "%2E%2E", "%2e%2E", "%2E%2e");

        static singlesegment = new Array<string>(".", "%2e", "%2E");

        static spesicalSymbols = new Array<string>("@", "%40", "#", "%23", "=", "%3D", ":", "%3A", "/", "%2F", ";", "%3B", "?", "%3F");

        static specialcharacter = new Array<string>('\0', '\t', '\n', '\r', ' ', '#', '%', '/', ':', '?', '@', '[', '\\', ']');

        static hexTable: Array<string> = new Array<string>(256);

        static {
            Helper.specialCharForBit.fill(0);
            let specialSymbolsTmp = ['#', '%', '/', ':', '?', '@', '[', '\\', ']', '<', '>', '^', '|'];
            for (let i = 0; i <= Helper.ASCII_32; i++) {
                Helper.specialCharForBit[i] = 1;
            }
            for (let i = 0; i < specialSymbolsTmp.length; i++) {
                Helper.specialCharForBit[specialSymbolsTmp[i].charCodeAt(0)] = 1;
            }
            Helper.specialCharForBit[Helper.ASCII_127] = 1;

            Helper.head.set("ftp:", 21);
            Helper.head.set("file:", -1);
            Helper.head.set("gopher:", 70);
            Helper.head.set("http:", 80);
            Helper.head.set("https:", 443);
            Helper.head.set("ws:", 80);
            Helper.head.set("wss:", 443);
            const hexNum = 16;
            for (let i = 0; i < 256; ++i) {
                Helper.hexTable[i] = "%" +
                    ((i < hexNum ? "0" : "") + Double.toString(i, hexNum))  // Convert to hexadecimal and handle leading zeros
                        .toUpperCase();
            }
        }

        // hostname, host
        static URL_ENCODED_PERCENT_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // 0x50 - 0x5F
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, // 0x70 - 0x7F
        ]);

        // username, password {'%25':'%'}
        static USERINFO_PERCENT_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // 0x50 - 0x5F
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, // 0x70 - 0x7F
        ]);

        // path {'%7C': '|', '%5B': '[', '%5D': ']', '%25': '%'} %5C '\\'
        static PATH_PERCENT_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, // 0x50 - 0x5F  // todo 5c
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, // 0x70 - 0x7F
        ]);

        // hash, fragment {'%7C': '|', '%5B': '[', '%5D': ']', '%7B': '{', '%7D': '}', '%60': '`', '%25': '%'}
        static FRAGMENT_PERCENT_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x50 - 0x5F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 0x70 - 0x7F
        ]);

        // query {'%7C': '|', '%5B': '[', '%5D': ']', '%7B': '{', '%7D': '}', '%60': '`', '%5E': '^', '%25': '%'}
        static QUERY_PERCENT_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0x30 - 0x3F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, // 0x50 - 0x5F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 0x70 - 0x7F
        ]);

        // params tostring encode
        static PARAMS_TOSTRING_SIGN_CHARS = new Int8Array([
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, // 0x20 - 0x2F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 0x30 - 0x3F
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // 0x50 - 0x5F
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, // 0x70 - 0x7F
        ]);

        static unhexTable = new Int8Array([
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0 - 15
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 16 - 31
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 32 - 47
            +0, +1, +2, +3, +4, +5, +6, +7, +8, +9, -1, -1, -1, -1, -1, -1, // 48 - 63
            -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 64 - 79
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 80 - 95
            -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 96 - 111
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 112 - 127
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 128 ...
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // ... 255
        ]);
    }

    function checkCharacter(input: string) {
        let inputLen = input.length;
        for (let i = 0; i < inputLen; i++) {
            let charPos: number = input.charCodeAt(i);
            if (charPos >= 0 && charPos < 128) {
                if (Helper.specialCharForBit[charPos]) {
                    return false;
                }
            }
        }
        return true;
    }

    function deleteC0OrSpace(input: string): string {
        if (input == "") {
            return "";
        }
        let i: number = 0;
        let strLen = input.length;
        let charCodeOfC0 = '\0'.charCodeAt(0);
        let charCodeOfSpace = ' '.charCodeAt(0);
        while (i < strLen) {
            let inputCharCode = input.charCodeAt(i);
            if (inputCharCode >= charCodeOfC0 && inputCharCode <= charCodeOfSpace) {
                i++;
                continue;
            }
            break;
        }
        let trimHead = input.substring(i);
        strLen = trimHead.length;
        if (strLen == 0) {
            return "";
        }
        for (i = strLen - 1; i >= 0; i--) {
            let inputCharCode = trimHead.charCodeAt(i);
            if (inputCharCode >= charCodeOfC0 && inputCharCode <= charCodeOfSpace) {
                continue;
            }
            break;
        }
        if (i < 0) {
            return "";
        }
        return trimHead.substring(0, i + 1);
    }

    function deleteTabOrNewline(input: string): string {
        let res = "";
        for (let i = 0; i < input.length; i++) {
            let ch = input[i];
            if (ch == c'\t' || ch == c'\n' || ch == c'\r') {
                continue;
            } else {
                res += ch;
            }
        }
        return res;
    }

    function isAlpha(ch: string): boolean {
        return ch.length == 1 && ((ch[0] >= c'a' && ch[0] <= c'z') || (ch[0] >= c'A' && ch[0] <= c'Z'));
    }

    function isNum(ch: string): boolean {
        return ch.length == 1 && (ch[0] >= c'0' && ch[0] <= c'9');
    }

    function isAlphaNum(ch: string): boolean {
        return isAlpha(ch) || isNum(ch);
    }

    function isSpecialScheme(scheme: string) {
        return Helper.head.has(scheme);
    }

    function isFileNotHost(input: string) {
        if (isAlpha(input.charAt(0)) && (input[1] == c':' || input[1] == c'|')) {
            return true;
        }
        return false;
    }

    function isHexDigit(ch: string): boolean {
        let charCode = ch.charCodeAt(0);
        return isNum(ch) || (charCode >= c'A' && charCode <= c'F') || (charCode >= c'a' && charCode <= c'f');
    }

    function DecodeSpecialChars(input: string): string {
        let temp: string = input;
        let len: number = temp.length;
        if (input.length == 0) {
            return temp;
        }
        let pos: number = temp.indexOf("%");
        while ((pos != -1) && (pos < len - 2)) {
            if (isHexDigit(temp.charAt(pos + 1)) && isHexDigit(temp.charAt(pos + 2))) {
                let subStr = temp.substring(pos + 1, pos + 3);
                let octNum: number = parseInt(subStr, 16);
                if (isNaN(octNum)) {
                    return temp;
                }

                let convertedChar = String.fromCharCode(octNum);
                temp = temp.slice(0, pos) + convertedChar + temp.slice(pos + 3);
                len = len - 2;
            }
            pos = temp.indexOf("%", pos + 1);
        }
        return temp;
    }

    function toHex(num: number): string {
        const hexDigits = "0123456789abcdef";
        let hexStr = "";
        while (num > 0) {
            const remainder = num % 16;
            hexStr = hexDigits[remainder] + hexStr;
            num = Math.floor(num / 16);
        }

        return hexStr || "0";
    }

    function dealIPv4(input: string): string {
        let temp = new Array<string>();
        let pos: number = input.lastIndexOf(':');
        let index: number = pos;
        let left = pos + 1;
        let hexVal = "";
        let val = "";
        while (true) {
            pos = input.indexOf(".", left);
            if (pos == -1) {
                break;
            }

            val = input.substring(left, pos);
            let num = parseInt(val, 10);
            if (isNaN(num) && num > 255) {
                return val;
            }

            hexVal = toHex(num);
            if (hexVal.length == 1) {
                hexVal = c'0' + hexVal;
            }
            temp.push(hexVal);
            left = pos + 1;
        }
        val = input.substring(left);
        let num = parseInt(val, 10);
        if (isNaN(num) && num > 255) {
            return val;
        }
        hexVal = toHex(num);
        if (hexVal.length == 1) {
            hexVal = c'0' + hexVal;
        }
        temp.push(hexVal);
        let res = input.substring(0, index);
        res = res + ":" + temp[0] + temp[1] + ":" + temp[2] + temp[3];
        return res;
    }

    function formatIPv6(input: string): string {
        let pos: number = input.indexOf("::");
        let index: number = pos;
        if (pos != -1) {
            let left: number = 0;
            let count: number = 0;
            while ((pos = input.indexOf(":", left)) != -1) {
                count += 1;
                left = pos + 1;
            }
            let size = 7 - (count - 2);
            let temp = "";
            for (let i = 0; i < size - 1; i++) {
                temp += ":0";
            }
            temp += ":";
            input = input.slice(0, index) + temp + input.substring(index + 2);
            if (index == 0) {
                input = "0" + input;
            }
        }
        return input;
    }

    function removeLeadingZeros(inputs: Array<string>) {
        let inputSize = inputs.length;
        for (let i = 0; i < inputSize; i++) {
            let strLen: number = inputs[i].length;
            let count: number = 0;
            let j = 0;
            for (j = 0; j < strLen; j++) {
                if (inputs[i][j] != c'0') {
                    break;
                }
                count++;
            }
            if (count == strLen) {
                inputs[i] = "0";
            } else if (count != 0) {
                inputs[i] = inputs[i].substring(j);
            }
        }
    }

    function zeroCompression(inputs: Array<string>): string {
        let maxIndex: number = 0;
        let maxSize: number = 0;
        let index: number = 0;
        let size: number = 0;
        let needZeroCompression: boolean = false;
        let inputSize: number = inputs.length;
        for (let i = 0; i < inputSize; i++) {
            index = i;
            size = 0;
            while (i < inputSize && inputs[i] == "0") {
                needZeroCompression = true;
                size++;
                i++;
            }
            if (maxSize < size) {
                maxSize = size;
                maxIndex = index;
            }
        }
        let res: string = "";
        for (let i = 0; i < inputSize; i++) {
            if (needZeroCompression && i == maxIndex) {
                if (maxIndex == 0) {
                    res += "::";
                } else {
                    res += ":";
                }
                i += maxSize - 1;
                continue;
            }
            res += inputs[i];
            i != (inputSize - 1) ? res += ":" : "";
        }
        return res;
    }

    function compress(input: string): string {
        let temp = new Array<string>();
        let pos: number = 0;
        let left: number = 0;
        while (true) {
            pos = input.indexOf(":", left);
            if (pos == -1) {
                break;
            }
            temp.push(input.substring(left, pos));
            left = pos + 1;
        }
        temp.push(input.substring(left));
        removeLeadingZeros(temp);
        let res: string = zeroCompression(temp);
        return res.toLowerCase();
    }

    function isRadix(input: string, radix: string): boolean {
        let len = input.length;
        for (let i = 0; i < len; i++) {
            if (radix.indexOf(input[i]) == -1) {
                return false;
            }
        }
        return true;
    }

    function isNumber(num: string, convension: number[]): boolean {
        let len: number = num.length;
        if (len >= 2 && num[0] == c'0' && (num[1] == c'x' || num[1] == c'X')) {
            convension[0] = 16;
            let subStr = num.substring(2);
            if (subStr.length == 0) {
                return true;
            }
            return isRadix(subStr, "0123456789abcdefABCDEF");
        } else if (len >= 1 && num[0] == c'0') {
            convension[0] = 8;
            let subStr = num.substring(1);
            if (subStr.length == 0) {
                return true;
            }
            return isRadix(subStr, "01234567");
        } else if (isRadix(num, "0123456789")) {
            convension[0] = 10;
            return true;
        }
        return false;
    }

    function BinaryConversion(num: string, radix: number): string {
        let val: number = 0;
        if (radix == 16) {
            if (num.substring(2).length == 0) {
                return "0";
            }
            val = parseInt(num, 16);
            if (isNaN(val)) {
                return num;
            }
            return Number.toString(val);
        } else if (radix == 8) {
            if (num.substring(1).length == 0) {
                return "0";
            }
            val = parseInt(num, 8);
            if (isNaN(val)) {
                return num;
            }
            return Number.toString(val);
        }
        return num;
    }

    function isFormatIPv4(nums: Array<string>): number {
        let len = nums.length;
        for (let i = 0; i < len; i++) {
            if (nums[i].length > 8) {
                return i;
            }
            if (nums[i].length > 0 && (parseInt(nums[i]) > 255)) {
                return i;
            }
        }
        return -1;
    }

    function splitNum(num: string, storeNumber: number[]): string {
        if (num.length > 8) {
            storeNumber[0] = num.length;
            return num;
        }
        let val: number = parseInt(num);
        let nums = new Array<string>();
        let res: string = "";
        while (val > 0) {
            let numConvert: number = val % 256;
            nums.push(Number.toString(numConvert));
            val = Math.floor(val / 256);
        }
        for (let i = nums.length - 1; i >= 0; i--) {
            res += nums[i] + ".";
        }

        storeNumber[0] = nums.length;
        return res.slice(0, res.length - 1);
    }

    class UrlInfo {
        cScheme: string = "";
        cFragment: string = "";
        cQuery: string = "";
        cPath = new Array<string>();
        cHost: string = "";
        cIsSpecialPath: boolean = false;
        cUsername: string = "";
        cPassword: string = "";
        cPort: number = -1;
    }

    class InnerURL {
        flags: boolean[] = [false, false, false, false, false, false, false, false, false, false, false, false];
        cScheme: string = "";
        cFragment: string = "";
        cQuery: string = "";
        cPath: Array<string> = new Array<string>();
        cHost: string = "";
        cIsSpecialPath: boolean = false;
        cUsername: string = "";
        cPassword: string = "";
        cPort: number = -1;

        private ipv4Array: Array<string> = new Array<string>();

        analysizeScheme(input: string): boolean {
            if (!isAlpha(input.charAt(0))) {
                this.flags[0] = true;
                return false;
            }
            let sizeLen: number = input.length;
            for (let i = 1; i < sizeLen - 1; i++) {
                if (!isAlphaNum(input.charAt(i)) && input[i] != c'+' && input[i] != c'-' && input[i] != c'.') {
                    this.flags[0] = true;
                    return false;
                }
            }
            this.cScheme = input.toLowerCase();
            if (isSpecialScheme(this.cScheme)) {
                this.flags[1] = true;
            }
            return true;
        }

        private analysizeFragment(fragment: string) {
            this.cFragment = fragment;
            this.flags[8] = true;
        }

        private analysizeQuery(query: string) {
            this.cQuery = query;
            this.flags[7] = true;
        }

        private analysizeFilePath(input: string) {
            let temp = new Array<string>();
            let pos: number = 0;
            while ((pos = input.indexOf('/')) != -1 || (pos = input.indexOf('\\')) != -1) {
                temp.push(input.substring(0, pos));
                input = input.substring(pos + 1);
            }
            temp.push(input);
            let tempLength: number = temp.length;
            for (let i = 0; i < tempLength; i++) {
                if (Helper.doubleSegment.includes(temp[i])) {
                    if (this.cPath.length == 1 && isFileNotHost(this.cPath[0]) && this.cPath[0].length == 2) {
                        this.cPath[0] = this.cPath[0].charAt(0) + ':';
                    } else if (this.cPath.length > 0) {
                        this.cPath.pop();
                    }
                    if (i == temp.length - 1) {
                        this.cPath.push("");
                    }
                    continue;
                }

                if (Helper.singlesegment.includes(temp[i])) {
                    if (i == temp.length - 1) {
                        this.cPath.push("");
                    }
                    continue;
                }
                this.cPath.push(temp[i]);
                this.flags[6] = true;
            }

            let it = this.cPath[0];
            if (isAlpha(it.charAt(0)) && (it[1] == c':' || it[1] == c'|')) {
                if (it.length == 2) {
                    this.cPath[0] = it[0] + ':';
                    this.flags[4] = false;
                    this.cHost = "";
                }
            }
        }

        private analysizeSpecialFile(input: string, pos: number) {
            let strHost: string = input.substring(0, pos);
            let strPath: string = input.substring(pos + 1);
            let special: boolean = true;
            let isFile = isFileNotHost(strHost);
            if (!isFile) {
                this.analysizeHost(strHost, special);
            } else if (!isFile && this.flags[0]) {
                return;
            }
            if (!isFile) {
                this.analysizeFilePath(strPath);
            } else {
                this.analysizeFile(input);
            }
        }

        private analysizeIPv6Host(input: string) {
            let regex: RegExp = new RegExp("(::|(:((:[0-9A-Fa-f]{1,4}){1,7}))|(([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|:))|(((:(:[0-9A-Fa-f]{1,4}){0,5}:)|(([0-9A-Fa-f]{1,4}:){1}(:[0-9A-Fa-f]{1,4}){0,4}:)|(([0-9A-Fa-f]{1,4}:){2}(:[0-9A-Fa-f]{1,4}){0,3}:)|(([0-9A-Fa-f]{1,4}:){3}(:[0-9A-Fa-f]{1,4}){0,2}:)|(([0-9A-Fa-f]{1,4}:){4}(:[0-9A-Fa-f]{1,4})?:)|(([0-9A-Fa-f]{1,4}:){5}:)|(([0-9A-Fa-f]{1,4}:){6}))((25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)))(%[a-zA-Z0-9._]+)?")
            if (!regex.test(input)) {
                this.flags[0] = true;
                return;
            }
            let pos: number = input.indexOf('.');
            if (pos != -1) {
                input = dealIPv4(input);
            }
            input = formatIPv6(input);
            input = compress(input);
            this.cHost = "[" + input + "]";
            this.flags[4] = true;
            this.flags[10] = true
        }

        private removalIPv4(inputStr: string): boolean {
            // this.ipv4Array = new Array<string>();
            let pos: number = 0;
            let left: number = 0;
            // let inputs = new Array<string>()
            pos = inputStr.indexOf(".", left);
            while (pos != -1) {
                this.ipv4Array.push(inputStr.substring(left, pos));
                left = pos + 1;
                pos = inputStr.indexOf(".", left);
            }

            if (left != inputStr.length) {
                this.ipv4Array.push(inputStr.substring(left));
            }

            let inputSize: number = this.ipv4Array.length;
            let res = new Array<string>();
            for (let i = 0; i < inputSize; i++) {
                let conversion: number[] = [0];
                if (isNumber(this.ipv4Array[i], conversion)) {
                    res.push(BinaryConversion(this.ipv4Array[i], conversion[0]));
                } else {
                    return false;
                }
            }
            this.ipv4Array = res;
            let isIPv4 = true;
            for (let i = 0; i < inputSize; i++) {
                if (res[i] == '') {
                    isIPv4 = false;
                    this.flags[0] = true;
                    if (i == inputSize - 1) {
                        res.push('');
                        this.flags[0] = false;
                    }
                }
            }
            return isIPv4;
        }

        private formatIPv4(nums: Array<string>) {
            let len: number = nums.length;
            let index: number = isFormatIPv4(nums);
            let res: string = "";
            if (index == -1) {
                for (let i = 0; i < len - 1; i++) {
                    res += nums[i] + '.';
                }
                for (let i = 0; i < (4 - len); i++) {
                    res += "0.";
                }
                res += nums[len - 1];
                this.cHost = res;
                this.flags[4] = true;
            } else if (index == (len - 1)) {
                for (let i = 0; i < (len - 1); i++) {
                    res += nums[i] + ".";
                }
                let storeNumber: number[] = [0];
                let temp: string = splitNum(nums[index], storeNumber);
                if (storeNumber[0] + (len - 1) > 4) {
                    this.flags[0] = true;
                    return;
                }
                for (let i = 0; i < (4 - (len - 1 + storeNumber[0])); i++) {
                    temp = "0." + temp;
                }
                this.cHost = res + temp;
                this.flags[4] = true;
            } else {
                this.flags[0] = true;
            }
        }

        private analysizeIPv4(input: string) {
            let isIPv4: boolean = false;
            isIPv4 = this.removalIPv4(input);
            let temp = this.ipv4Array;
            let tempLen: number = temp.length;
            let lastSize: number = temp[tempLen - 1].length;
            if (isIPv4 && lastSize > 8) {
                this.flags[0] = true;
                return;
            }
            let res: string = "";
            for (let i = 0; i < tempLen; i++) {
                res += temp[i];
                if (i != (tempLen - 1)) {
                    res += '.';
                }
            }
            if (isIPv4) {
                if (tempLen > 4) {
                    res = res.toLowerCase();
                    this.cHost = res;
                    this.flags[4] = true;
                } else if (tempLen == 4) {
                    if (isFormatIPv4(temp) == -1) {
                        this.cHost = res;
                        this.flags[4] = true;
                    } else {
                        this.flags[0] = true;
                    }
                } else {
                    this.formatIPv4(temp);
                }
            } else {
                res = res.toLowerCase();
                this.cHost = res;
                this.flags[4] = true;
            }
        }

        private analysizeOpaqueHost(input: string) {
            let inputSize: number = input.length;
            for (let i = 0; i < inputSize; i++) {
                let ch = input[i];
                let result = Helper.specialcharacter.indexOf(ch);
                if ((ch != c'%') && (result != -1)) {
                    this.flags[0] = true;
                    return;
                }
            }
            this.cHost = input;
            this.flags[4] = true;
        }

        analysizeHost(input: string, special: boolean) {
            if (input.length == 0) {
                this.flags[0] = true;
                return;
            }
            let inputLength = input.length;
            if (input[0] == c'[') {
                if (input[inputLength - 1] == c']') {
                    input = input.substring(1, inputLength - 1);
                    this.analysizeIPv6Host(input);
                    return;
                }
                this.flags[0] = true;
                return;
            }
            if (!special) {
                this.analysizeOpaqueHost(input);
                return;
            }
            let decodedInput = DecodeSpecialChars(input);
            if (!checkCharacter(decodedInput)) {
                this.flags[0] = true;
                return;
            }
            this.analysizeIPv4(decodedInput);
        }

        private analysizeFile(input: string) {
            let special: boolean = true;
            if ((input[0] == c'/' || input[0] == c'\\') && (input[1] == c'/' || input[1] == c'\\')) {
                let temp: string = input.substring(2);
                let pos: number = 0;
                if (((pos = temp.indexOf('/')) != -1 || (pos = temp.indexOf('\\')) != -1) && pos == 0) {
                    temp = temp.substring(1);
                    this.analysizeFilePath(temp);
                } else if (((pos = temp.indexOf('/')) != -1 || (pos = temp.indexOf('\\')) != -1) && pos != 0) {
                    this.analysizeSpecialFile(temp, pos);
                } else {
                    if (temp.length != 0 && this.flags[0]) {
                        this.analysizeHost(temp, special);
                    } else if (temp.length != 0 && !this.flags[0]) {
                        this.analysizeHost(temp, special);
                        return;
                    }
                }
            } else {
                if (input[0] == c'/' || input[0] == c'\\') {
                    input = input.substring(1);
                }
                this.analysizeFilePath(input);
            }
        }

        private analysizeUsernameAndPasswd(strHost: string): string {
            let pos: number = strHost.length - 1;
            for (; pos >= 0; pos--) {
                if (strHost[pos] == c'@') {
                    break;
                }
            }
            let userAndPasswd = strHost.substring(0, pos);
            strHost = strHost.substring(pos + 1);
            if (userAndPasswd.length == 0) {
                return strHost;
            }
            if (userAndPasswd.indexOf('@') != -1) {
                while (true) {
                    let posTemp: number = userAndPasswd.indexOf('@');
                    if (posTemp != -1) {
                        userAndPasswd = userAndPasswd.replace('@', "%40");
                    } else {
                        break;
                    }
                }
            }
            if (userAndPasswd.indexOf(':') != -1) {
                let position: number = userAndPasswd.indexOf(':');
                let user: string = userAndPasswd.substring(0, position);
                let keyWord: string = userAndPasswd.substring(position + 1);
                if (user.length != 0) {
                    this.cUsername = user;
                    this.flags[2] = true;
                }
                if (keyWord.length != 0) {
                    this.cPassword = keyWord;
                    this.flags[3] = true;
                }
            } else {
                this.cUsername = userAndPasswd;
                this.flags[2] = true;
            }
            return strHost;
        }

        analysizePort(port: string) {
            if (port.length == 0) {
                return;
            }
            for (let i = 0; i < port.length; i++) {
                if (!isNum(port.charAt(i))) {
                    this.flags[0] = true;
                    return;
                }
            }
            if (port.length >= 6) {
                this.flags[0] = true;
                return;
            }
            let it: number = parseInt(port);
            const maxPort: number = 65535;
            if (it > maxPort) {
                this.flags[0] = true;
                return;
            }
            this.flags[5] = true;
            for (let key of Helper.head.keys()) {
                let value = Helper.head.get(key);
                if (key == this.cScheme && value == it) {
                    this.cPort = -1;
                    this.flags[5] = false;
                    return;
                }
            }
            this.cPort = it;
        }

        analysizePath(path: string, special: boolean) {
            let temp = new Array<string>();
            let pos: number = 0;
            while ((pos = path.indexOf('/')) != -1 || ((pos = path.indexOf('\\')) != -1 && special)) {
                temp.push(path.substring(0, pos));
                path = path.substring(pos + 1);
            }
            temp.push(path);
            let tempLen: number = temp.length;
            for (let it = 0; it < tempLen; it++) {
                let result: number = Helper.doubleSegment.indexOf(temp[it]);
                if (result != -1) {
                    if (this.cPath.length == 0 && it == (tempLen - 1)) {
                        this.cPath.push("");
                        this.flags[6] = true;
                    }
                    if (this.cPath.length == 0) {
                        continue;
                    }
                    this.cPath.pop();
                    if (it == (tempLen - 1)) {
                        this.cPath.push("");
                        this.flags[6] = true;
                    }
                    continue;
                }

                result = Helper.singlesegment.indexOf(temp[it]);
                if (result != -1 && it == (tempLen - 1)) {
                    this.cPath.push("");
                    this.flags[6] = true;
                    continue;
                }
                if (result == -1) {
                    this.cPath.push(temp[it]);
                    this.flags[6] = true;
                }
            }
        }

        private parsingHostAndPath(input: string, pos: number) {
            let special: boolean = true;
            let inputLen: number = input.length;
            for (pos = 0; pos < inputLen; pos++) {
                if (input[pos] == c'/' || input[pos] == c'\\') {
                    break;
                }
            }
            let strHost: string = input.substring(0, pos);
            let strPath: string = input.substring(pos + 1);
            if (strHost.indexOf('@') != -1) {
                strHost = this.analysizeUsernameAndPasswd(strHost);
            }
            if (strHost.length == 0) {
                this.flags[0] = true;
                return;
            }
            if (strHost[strHost.length - 1] != c']' && (strHost.lastIndexOf(':') != -1)) {
                pos = strHost.lastIndexOf(':');
                if (pos != -1) {
                    let port: string = strHost.substring(pos + 1);
                    strHost = strHost.substring(0, pos);
                    this.analysizePort(port);
                }
            }
            if (strHost[strHost.length - 1] != c']' && strHost.lastIndexOf(':') != -1 && this.flags[0]) {
                return;
            }
            this.analysizeHost(strHost, special);
            this.analysizePath(strPath, special);
        }

        private analysizeOnlyHost(input: string, pos: number) {
            let strHost: string = input;
            if (strHost.indexOf('@') != -1) {
                strHost = this.analysizeUsernameAndPasswd(strHost);
            }
            if (strHost.length == 0) {
                this.flags[0] = true;
                return;
            }
            if (strHost[strHost.length - 1] != c']') {
                if ((pos = strHost.lastIndexOf(':')) != -1) {
                    let port: string = strHost.substring(pos + 1);
                    strHost = strHost.substring(0, pos);
                    this.analysizePort(port);
                }
                if ((pos = strHost.lastIndexOf(':')) != -1 && this.flags[0]) {
                    return;
                }
            }
            this.analysizeHost(strHost, true);
        }

        private analysizeFilescheme(input: string) {
            let strPath: string = this.cScheme + input;
            this.cScheme = "file:";
            this.flags[1] = true;
            this.analysizeFilePath(strPath);
        }

        private analysizeStrHost(input: string): string {
            if (input.indexOf('@') != -1) {
                input = this.analysizeUsernameAndPasswd(input);
            }
            if (input.length == 0) {
                this.flags[0] = true;
            }
            return input;
        }

        private analysizeHostPath(input: string): string {
            let pos: number = 0;
            if (input[input.length - 1] != c']' && (pos = input.lastIndexOf(':')) != -1) {
                let port: string = input.substring(pos + 1);
                input = input.substring(0, pos);
                this.analysizePort(port);
                if (this.flags[0]) {
                    return input;
                }
            }
            return input;
        }

        private analysizeInfoPath(input: string) {
            this.flags[9] = true;
            if (this.cPath.length == 0) {
                this.cPath.push("");
            }
            this.cPath[0] = input;
            this.flags[6] = true;
            return;
        }

        private analysizeNoDefaultProtocol(input: string) {
            if (this.cScheme.length == 2) {
                this.analysizeFilescheme(input);
                return;
            }
            if (input[0] == c'/' && input[1] == c'/' && input[2] != c'/') {
                let hostAndPath: string = input.substring(2);
                if (hostAndPath.length == 0) {
                    return;
                }
                let i: number = 0;
                let special: boolean = false;
                let strHost: string = "";
                if (hostAndPath.indexOf('/') != -1) {
                    i = hostAndPath.indexOf('/');
                    strHost = hostAndPath.substring(0, i);
                    let strPath: string = hostAndPath.substring(i + 1);
                    if (strHost.indexOf('@') != -1) {
                        strHost = this.analysizeUsernameAndPasswd(strHost);
                    }
                    if (strHost.length == 0) {
                        this.flags[0] = true;
                        return;
                    }
                    let pos: number = 0;
                    if (strHost[strHost.length - 1] != c']' && (pos = strHost.lastIndexOf(':')) != -1) {
                        let port: string = strHost.substring(pos + 1);
                        strHost = strHost.substring(0, pos);
                        this.analysizePort(port);
                    }
                    if (strHost[strHost.length - 1] != c']' && (pos = strHost.lastIndexOf(':')) != -1 && this.flags[0]) {
                        return;
                    }
                    this.analysizeHost(strHost, special);
                    this.analysizePath(strPath, special);
                } else {
                    strHost = hostAndPath;
                    strHost = this.analysizeStrHost(strHost);
                    strHost = this.analysizeHostPath(strHost);
                    this.analysizeHost(strHost, special);
                }
            } else if (input[0] == c'/' && input[1] == c'/') {
                let strOfPath = input.substring(1);
                this.analysizePath(strOfPath, false);
            } else {
                this.analysizeInfoPath(input);
            }
        }

        private analysizeHostAndPath(input: string) {
            if (this.flags[1]) {
                let pos: number = 0;
                let inputLen: number = input.length;
                while (pos < inputLen) {
                    if (input[pos] == c'/' || input[pos] == c'\\') {
                        pos++;
                        continue;
                    }
                    break;
                }
                input = input.substring(pos);
                if (input.length == 0) {
                    this.flags[0] = true;
                    return;
                } else if (input.indexOf('/') != -1 || input.indexOf('\\') != -1) {
                    this.parsingHostAndPath(input, pos);
                } else if (input.length != 0 && input.indexOf('/') == -1 && input.indexOf('\\') == -1) {
                    this.analysizeOnlyHost(input, pos);
                }
            } else {
                let inputLen: number = input.length;
                if (inputLen > 0) {
                    this.cIsSpecialPath = input[0] != c'/' ? true : false;
                }
                if (this.flags[11] && inputLen == 2) {
                    return;
                }
                this.analysizeNoDefaultProtocol(input);
            }
        }

        initOnlyInput(input: string): string {
            if (input.length == 0) {
                this.flags[0] = true;
                return input;
            }
            let tempInput = input;
            if (tempInput.indexOf(':') != -1) {
                let pos = tempInput.indexOf(':');
                pos++;
                this.flags[11] = (input[pos] == c'/' && input[pos + 1] == c'/');
                let scheme: string = tempInput.substring(0, pos);
                if (!this.analysizeScheme(scheme)) {
                    return tempInput;
                }
                if (tempInput.indexOf('#') != -1) {
                    let posTemp = tempInput.indexOf('#');
                    let fragment: string = tempInput.substring(posTemp);
                    this.analysizeFragment(fragment);
                    tempInput = tempInput.substring(0, posTemp);
                }
                if (tempInput.indexOf('?') != -1) {
                    let posTemp = tempInput.indexOf('?');
                    let query: string = tempInput.substring(posTemp);
                    this.analysizeQuery(query);
                    tempInput = tempInput.substring(0, posTemp);
                }
                let restInput: string = tempInput.substring(pos);
                if (this.cScheme == "file:") {
                    this.analysizeFile(restInput);
                } else {
                    this.analysizeHostAndPath(restInput);
                }
            } else {
                this.flags[0] = true;
            }

            return tempInput;
        }

        analysizeInput(input: string): string {
            let pos: number = 0;
            if (input.indexOf('#') != -1) {
                pos = input.indexOf('#');
                let fragment: string = input.substring(pos);
                this.analysizeFragment(fragment);
                input = input.substring(0, pos);
            }
            if (input.indexOf('?') != -1) {
                pos = input.indexOf('?');
                let query: string = input.substring(pos);
                this.analysizeQuery(query);
                input = input.substring(0, pos);
            }
            let special: boolean = this.flags[1] ? true : false;
            // let pathStr: string = input;
            this.analysizePath(input, special);
            return input;
        }

        toolHasBase(input: string, strInput: string): string {
            if (input.length != 0 && input[0] == c'/') {
                strInput = input.substring(1);
                return this.analysizeInput(strInput);
            } else if (input.length != 0 && input[0] != c'/') {
                return this.analysizeInput(strInput);
            }
            return strInput;
        }

        parseSingleUrl(inputUrl: string): string {
            let trimedStr = deleteC0OrSpace(inputUrl);
            return this.initOnlyInput(trimedStr);
        }

        constructor() { }

        getHostName(): string {
            let temp = "";
            if (this.flags[4]) {
                if (!this.flags[10]) {
                    temp = encodePercentEncoding(this.cHost, Helper.URL_ENCODED_PERCENT_SIGN_CHARS);
                } else {
                    temp = this.cHost;
                }
            }
            return temp;
        }

        setHostName(input: string) {
            if (this.flags[9]) {
                return;
            }
            let strHost: string = input;
            let len: number = strHost.length;
            for (let pos = 0; pos < len; pos++) {
                if ((strHost[pos] == c':') || (strHost[pos] == c'?') || (strHost[pos] == c'#') || (strHost[pos] == c'/' || strHost[pos] == c'\\')) {
                    strHost = strHost.substring(0, pos);
                    break;
                }
            }
            if (strHost.length == 0) {
                return;
            }
            let special: boolean = Helper.head.has(this.cScheme);
            let tempUrl: InnerURL = new InnerURL();
            tempUrl.analysizeHost(strHost, special);
            if (tempUrl.flags[4]) {
                if (this.cScheme == "file:" && (tempUrl.cHost == "localhost")) {
                    tempUrl.cHost = "";
                }
                this.cHost = tempUrl.cHost;
                this.flags[4] = true;
            }
        }

        setUserName(input: string) {
            if (input.length == 0) {
                this.cUsername = "";
                this.flags[2] = false;
            } else {
                let username: string = encodePercentEncoding(input, Helper.USERINFO_PERCENT_SIGN_CHARS);
                let len: number = Helper.spesicalSymbols.length - 2;
                for (let i = 0; i <= len; i += 2) {
                    // ReplaceSpecialSymbols
                    username = username.replaceAll(Helper.spesicalSymbols[i], Helper.spesicalSymbols[i + 1]);
                }
                this.cUsername = username;
                this.flags[2] = true;
            }
        }

        setPassword(input: string) {
            if (input.length == 0) {
                this.cPassword = "";
                this.flags[3] = false;
            } else {
                let keyWord: string = encodePercentEncoding(input, Helper.USERINFO_PERCENT_SIGN_CHARS);
                let len: number = Helper.spesicalSymbols.length - 2;
                for (let i = 0; i <= len; i += 2) {
                    keyWord = keyWord.replaceAll(Helper.spesicalSymbols[i], Helper.spesicalSymbols[i + 1]);
                }
                this.cPassword = keyWord;
                this.flags[3] = true;
            }
        }

        setScheme(input: string) {
            let strInput: string = input;
            let special: boolean = Helper.head.has(this.cScheme);
            let inputIsSpecial: boolean = Helper.head.has(input);
            if ((special != inputIsSpecial) || ((input == "file") && (this.flags[2] || this.flags[3] || this.flags[5]))) {
                return;
            }
            let tempUrl: InnerURL = new InnerURL();
            if (tempUrl.analysizeScheme(strInput)) {
                if (tempUrl.flags[1]) {
                    this.flags[1] = true;
                }
                this.cScheme = tempUrl.cScheme;
            }
        }

        setFragment(input: string) {
            let temp: string = "";
            if (input.length == 0) {
                this.cFragment = "";
                this.flags[8] = false;
            } else {
                if (input[0] != c'#') {
                    temp = '#';
                    temp += encodePercentEncoding(input, Helper.FRAGMENT_PERCENT_SIGN_CHARS);
                } else {
                    temp = encodePercentEncoding(input, Helper.FRAGMENT_PERCENT_SIGN_CHARS);
                }
                this.analysizeFragment(temp);
            }
        }

        setSearch(input: string) {
            let temp: string = "";
            if (input.length == 0) {
                this.cQuery = "";
                this.flags[7] = false;
            } else {
                if (input[0] != c'?') {
                    temp = "?";
                    temp += input;
                } else {
                    temp = input;
                }
                let oldStr = "#";
                let newStr = "%23";
                temp = temp.replaceAll(oldStr, newStr);
                this.analysizeQuery(temp);
            }
        }

        setEncodeSearch(input: string) {
            this.setSearch(encodePercentEncoding(input, Helper.QUERY_PERCENT_SIGN_CHARS));
        }

        setHost(input: string) {
            if (input.length == 0 || this.flags[9]) {
                return;
            }
            let strHost: string = input;
            let port: string = "";
            // splitString
            let strLen: number = input.length;
            for (let pos = 0; pos < strLen; pos++) {
                if ((input[pos] == c':') || (input[pos] == c'?') || (input[pos] == c'#') || (input[pos] == c'/') || (input[pos] == c'\\')) {
                    strHost = input.substring(0, pos);
                    if (input[pos] == c':') {
                        pos++;
                        port = input.substring(pos);
                    }
                    break;
                }
            }

            if (strHost.length == 0) {
                return;
            }
            let special: boolean = Helper.head.has(this.cScheme);
            let tempUrl: InnerURL = new InnerURL();
            tempUrl.analysizeHost(strHost, special);
            if (tempUrl.flags[4]) {
                if (this.cScheme == "file:" && tempUrl.cHost == "localhost") {
                    tempUrl.cHost = "";
                }
                this.cHost = tempUrl.cHost;
                this.flags[4] = true;
            } else {
                return;
            }
            if (port.length > 0) {
                let portLen: number = port.length;
                for (let pos = 0; pos < portLen; pos++) {
                    if (port[pos] == c'?' || port[pos] == c'#' || port[pos] == c'/' || port[pos] == c'\\') {
                        port = port.substring(0, pos);
                        break;
                    }
                }
                if (port.length > 0) {
                    let tempPortUrl: InnerURL = new InnerURL();
                    tempPortUrl.analysizePort(port);
                    if (tempPortUrl.flags[5]) {
                        this.flags[5] = true;
                        this.cPort = tempPortUrl.cPort;
                    }
                }
            }
        }

        setPort(input: string) {
            let port: string = input;
            let portLen: number = port.length;
            for (let pos = 0; pos < portLen; pos++) {
                if ((port[pos] == c'?') || (port[pos] == c'#') || (port[pos] == c'/') || (port[pos] == c'\\')) {
                    port = port.substring(0, pos);
                    break;
                }
            }
            if (port.length > 0) {
                let tempUrl: InnerURL = new InnerURL();
                tempUrl.analysizePort(port);
                if (tempUrl.flags[5]) {
                    this.flags[5] = true;
                    this.cPort = tempUrl.cPort;
                }
            }
        }

        setHref(input: string) {
            let str: string = input;
            str = deleteC0OrSpace(str);
            let newUrl: InnerURL = new InnerURL();
            newUrl.initOnlyInput(str);
            this.flags = newUrl.flags;
            if (!newUrl.flags[0]) {
                this.flags = newUrl.flags;
                this.cFragment = newUrl.cFragment;
                this.cScheme = newUrl.cScheme;
                this.cQuery = newUrl.cQuery;
                this.cPath = newUrl.cPath;
                this.cHost = newUrl.cHost;
                this.cIsSpecialPath = newUrl.cIsSpecialPath;
                this.cUsername = newUrl.cUsername;
                this.cPassword = newUrl.cPassword;
                this.cPort = newUrl.cPort;
            }
        }

        setPath(input: string) {
            let strPath: string = encodePercentEncoding(input, Helper.PATH_PERCENT_SIGN_CHARS);
            if (this.flags[9] || strPath.length == 0) {
                return;
            }
            let oldStr: string = "%3A";
            let newStr: string = ":";
            strPath = strPath.replaceAll(oldStr, newStr);
            let special: boolean = Helper.head.has(this.cScheme);
            if (this.cScheme == "file:") {
                let tempUrl: InnerURL = new InnerURL();
                if ((strPath[0] == c'/') || (strPath[0] == c'\\' && this.flags[1])) {
                    strPath = strPath.substring(1);
                }
                tempUrl.analysizeFilePath(strPath);
                if (tempUrl.flags[6]) {
                    this.cPath = tempUrl.cPath;
                    this.flags[6] = true;
                }
            } else {
                let tempUrl: InnerURL = new InnerURL();
                if ((strPath[0] == c'/') || (strPath[0] == c'\\' && this.flags[1])) {
                    strPath = strPath.substring(1);
                }
                tempUrl.analysizePath(strPath, special);
                if (tempUrl.flags[6]) {
                    this.cPath = tempUrl.cPath;
                    this.flags[6] = true;
                }
            }
        }

        getSearch(): string {
            if (this.flags[7] && (this.cQuery.length != 1)) {
                return this.cQuery;
            }
            return "";
        }

        getEncodeSearch(): string {
            if (this.flags[7] && (this.cQuery.length != 1)) {
                return encodePercentEncoding(this.cQuery, Helper.QUERY_PERCENT_SIGN_CHARS);
            }
            return "";
        }

        getUsername(): string {
            if (this.flags[2]) {
                return encodePercentEncoding(this.cUsername, Helper.USERINFO_PERCENT_SIGN_CHARS);
            }
            return "";
        }

        getPassword(): string {
            if (this.flags[3]) {
                return encodePercentEncoding(this.cPassword, Helper.USERINFO_PERCENT_SIGN_CHARS);
            }
            return "";
        }

        getFragment(): string {
            if (this.flags[8] && (this.cFragment.length != 1)) {
                return encodePercentEncoding(this.cFragment, Helper.FRAGMENT_PERCENT_SIGN_CHARS);
            }
            return "";
        }

        getScheme(): string {
            if (this.cScheme.length != 0) {
                return this.cScheme;
            }
            return "";
        }

        getPath(): string {
            let temp: string = "/";
            if (this.cIsSpecialPath) {
                temp = "";
            }
            if (this.flags[6]) {
                let len: number = this.cPath.length;
                for (let i = 0; i < len; i++) {
                    if (i < len - 1) {
                        temp += this.cPath[i] + "/";
                    } else {
                        temp += this.cPath[i];
                    }
                }
            } else {
                let special: boolean = Helper.head.has(this.cScheme);
                if (!special) {
                    temp = "";
                }
            }
            temp = encodePercentEncoding(temp, Helper.PATH_PERCENT_SIGN_CHARS);
            return temp;
        }

        getPort(): string {
            if (this.flags[5]) {
                return Number.toString(this.cPort);
            }
            return "";
        }

        getHost(): string {
            let temp: string = this.cHost;
            if (this.flags[5]) {
                temp += ":";
                temp += Number.toString(this.cPort);
            }
            if (!this.flags[10]) {
                temp = encodePercentEncoding(temp, Helper.URL_ENCODED_PERCENT_SIGN_CHARS);
            }
            return temp;
        }

        getIsIpv6(): boolean {
            return this.flags[10] ? true : false;
        }

        hasDoubleSlash(): boolean {
            return this.flags[11];
        }
    }

    function baseIntoUrl(base: InnerURL, res: InnerURL, inputIsEmpty: boolean) {
        res.cScheme = base.cScheme;
        res.flags[1] = base.flags[1];

        res.cHost = base.cHost;
        res.flags[4] = true

        res.cUsername = base.cUsername;
        res.flags[2] = base.flags[2];

        res.cPassword = base.cPassword;
        res.flags[3] = base.flags[3];

        res.cPort = base.cPort;
        res.flags[5] = base.flags[5];

        if (inputIsEmpty) {
            res.cPath = base.cPath;
            res.flags[6] = base.flags[6];

            res.cQuery = base.cQuery;
            res.flags[7] = base.flags[7];

            res.cFragment = base.cFragment;
            res.flags[8] = base.flags[8];
        }
        res.flags[9] = base.flags[9];
        res.flags[10] = base.flags[10];
    }

    function shorteningPath(resUrl: InnerURL, baseUrl: InnerURL, isFile: boolean) {
        if (baseUrl.cPath.length == 0) {
            return;
        }
        if (resUrl.cPath.length == 1 && resUrl.cPath[0].length == 0) {
            resUrl.cPath.pop();
            return;
        }
        if ((baseUrl.cPath.length == 1) && isFile && isAlpha(baseUrl.cPath[0].charAt(0)) && (baseUrl.cPath[0].charAt(1) == c':')) {
            return;
        }
        baseUrl.cPath.pop();
    }

    function basePathToString(base: InnerURL): string {
        let temp: string = "";
        let len: number = base.cPath.length;
        for (let i = 0; i < len; i++) {
            if (i < len - 1) {
                temp += base.cPath[i] + "/";
            } else {
                temp += base.cPath[i];
            }
        }
        return temp;
    }

    function setEncodedData(dst: URL, src: InnerURL) {
        dst.cSearch = src.getEncodeSearch();
        dst.cUsername = src.getUsername();
        dst.cPassword = src.getPassword();
        dst.cHostname = src.getHostName();
        dst.cHost = src.getHost();
        dst.cHash = src.getFragment();
        dst.cProtocol = src.getScheme();
        dst.cPathname = src.getPath();
        dst.cPort = src.getPort();
        dst.cOrigin = src.getScheme() + '//' + src.getHost();
        dst.cParams = new URLParams(src.getEncodeSearch());
        dst.cParams.parentUrl = dst;
        dst.cHasDoubleSlash = src.hasDoubleSlash();
        dst.setHref();
    }

    function containIllegalCode(str: string): Boolean {
        const unpairedSurrogateRe = new RegExp("(?:[^\uD800-\uDBFF]|^)[\uDC00-\uDFFF]|[\uD800-\uDBFF](?![\uDC00-\uDFFF])");
        const regex = new RegExp(unpairedSurrogateRe);
        return regex.test(str)
    }

    function fixIllegalString(str: string): string {
        if (containIllegalCode(str)) {
            return str.toWellFormed();
        } else {
            return str;
        }
    }

    export class URL {

        urlInner: InnerURL = new InnerURL();

        cHref: string = '';
        cSearch: string = '';
        cOrigin: string = '';
        cUsername: string = '';
        cPassword: string = '';
        cHostname: string = '';
        cHost: string = '';
        cHash: string = '';
        cProtocol: string = '';
        cPathname: string = '';
        cPort: string = '';
        cHasDoubleSlash: boolean = false;
        cParams: URLParams = new URLParams();

        constructor() { }

        static parseURL(inputUrl: string, inputBase?: string | URL): URL {
            let nativeUrl: URL;

            if (!inputBase) {
                nativeUrl = URL.parseSingleInput(fixIllegalString(inputUrl));
            } else {
                if (typeof inputBase == 'string') {
                    let inputBaseStr = inputBase as string;
                    nativeUrl = URL.parseBaseInput(fixIllegalString(inputUrl), fixIllegalString(inputBaseStr));
                } else {
                    nativeUrl = URL.parseBaseUrl(fixIllegalString(inputUrl), inputBase as URL);
                }
            }
            if (nativeUrl.urlInner.flags[0]) {
                throw createBusinessError(SyntaxErrorCodeId,'Syntax Error. Invalid Url string.');
            }
            setEncodedData(nativeUrl, nativeUrl.urlInner);
            return nativeUrl;
        }

        toString(): string {
            return this.cHref;
        }

        get protocol(): string {
            return this.cProtocol;
        }

        set protocol(scheme: string) {
            if (scheme.length == 0) {
                return;
            }
            if (this.cProtocol == "file:" && (this.cHost == "" || this.cHost == null)) {
                return;
            }
            this.urlInner.setScheme(scheme);
            this.cProtocol = this.urlInner.getScheme();
            this.cHost = this.urlInner.getHost();
            this.cPort = this.urlInner.getPort();
            this.setHref();
        }

        get origin(): string {
            let kOpaqueOrigin: string = 'null';
            switch (this.cProtocol) {
                case 'ftp:':
                case 'gopher:':
                case 'http:':
                case 'https:':
                case 'ws:':
                case 'wss:':
                    return this.cProtocol + '//' + this.cHost;
            }
            return kOpaqueOrigin;
        }

        get username(): string {
            return this.cUsername;
        }

        set username(input: string) {
            if (this.cHost == null || this.cHost == '' || this.cProtocol == 'file:') {
                return;
            }
            this.urlInner.setUserName(fixIllegalString(input));
            this.cUsername = this.urlInner.getUsername();
            this.setHref();
        }

        get password(): string {
            return this.cPassword;
        }

        set password(input: string) {
            if (this.cHost == null || this.cHost == '' || this.cProtocol == 'file:') {
                return;
            }
            this.urlInner.setPassword(fixIllegalString(input));
            this.cPassword = this.urlInner.getPassword();
            this.setHref();
        }

        get hash(): string {
            return this.cHash;
        }

        set hash(fragment: string) {
            this.urlInner.setFragment(fixIllegalString(fragment));
            this.cHash = this.urlInner.getFragment();
            this.setHref();
        }

        get search(): string {
            return this.cSearch;
        }

        set search(query: string) {
            this.urlInner.setEncodeSearch(fixIllegalString(query));
            this.cSearch = this.urlInner.getEncodeSearch();
            this.cParams.updateParams(this.cSearch);
            this.setHref();
        }

        get hostname(): string {
            return this.cHostname;
        }

        set hostname(hostname: string) {
            this.urlInner.setHostName(hostname);
            this.cHostname = this.urlInner.getHostName();
            this.cHost = this.urlInner.getHost();
            this.setHref();
        }

        get host(): string {
            return this.cHost;
        }

        set host(host: string) {
            this.urlInner.setHost(host);
            this.cHost = this.urlInner.getHost();
            this.cHostname = this.urlInner.getHostName();
            this.cPort = this.urlInner.getPort();
            this.setHref();
        }

        get port(): string {
            return this.cPort;
        }

        set port(port: string) {
            if (this.cHost == '' || this.cProtocol == 'file:' || port == '') {
                return;
            }
            this.urlInner.setPort(port);
            this.cPort = this.urlInner.getPort();
            this.cHost = this.urlInner.getHost();
            this.setHref();
        }

        get pathname(): string {
            return this.cPathname;
        }

        set pathname(path: string) {
            this.urlInner.setPath(fixIllegalString(path));
            this.cPathname = this.urlInner.getPath();
            this.setHref();
        }

        get params(): URLParams {
            return this.cParams;
        }

        toJSON(): string {
            return this.cHref;
        }

        get href(): string {
            return this.cHref;
        }

        set href(href: string) {
            this.urlInner.setHref(href);
            if (!this.urlInner.flags[0]) {
                setEncodedData(this, this.urlInner);
            } else {
                throw createBusinessError(SyntaxErrorCodeId,'Syntax Error. Invalid Url string.');
            }
        }

        setHref(): void {
            let temp: string = this.cProtocol;
            if (this.cHostname != "" || this.cProtocol == "file:" || this.cHasDoubleSlash) {
                temp += "//";
            }
            if (this.cHostname != "") {
                if (this.cPassword != "" || this.cUsername != "") {
                    if (this.cUsername != "") {
                        temp += this.cUsername;
                    }
                    if (this.cPassword != "") {
                        temp += ":";
                        temp += this.cPassword;
                    }
                    temp += "@";
                }
                temp += this.cHostname;
                if (this.cPort != "") {
                    temp += ":";
                    temp += this.cPort;
                }
            }
            temp += this.cPathname;
            if (this.cSearch) {
                temp += this.cSearch;
            }
            if (this.cHash) {
                temp += this.cHash;
            }
            this.cHref = temp;
        }

        static parseSingleInput(input: string): URL {
            let result = new URL();
            let url = new InnerURL();
            url.parseSingleUrl(input);
            result.urlInner = url;

            return result;
        }

        static parseBaseInput(input: string, base: string): URL {
            let result = new URL();
            let possibleBaseURL = new InnerURL();
            let resUrl = new InnerURL();
            let strInput = input;
            if (base.length == 0) {
                possibleBaseURL.flags[0] = true
            }
            possibleBaseURL.parseSingleUrl(base);
            if (possibleBaseURL.flags[0]) {
                resUrl.flags[0] = true
                result.urlInner = resUrl;
                return result;
            } else if (!possibleBaseURL.flags[0]) {
                strInput = resUrl.parseSingleUrl(strInput);
                if (!resUrl.flags[0]) {
                    result.urlInner = resUrl;
                    return result;
                }
                if ((input[0] == c'/') && (input[1] == c'/' || (input[1] == c'\\' && possibleBaseURL.flags[1]))) {
                    let newInput: string = possibleBaseURL.cScheme + input;
                    resUrl.flags[0] = false
                    resUrl.parseSingleUrl(newInput);
                    result.urlInner = resUrl;
                    return result;
                }
                if (!possibleBaseURL.flags[9]) {
                    resUrl.flags[0] = false
                    baseIntoUrl(possibleBaseURL, resUrl, input.length == 0);
                    strInput = resUrl.toolHasBase(input, strInput);
                    if (input.length != 0 && input[0] != c'/' && resUrl.cPath.length == 0) {
                        resUrl.cPath = possibleBaseURL.cPath;
                        resUrl.flags[6] = possibleBaseURL.flags[6];
                    }
                    if (input.length != 0 && input[0] != c'/' && resUrl.cPath.length != 0) {
                        let isFile: boolean = ((resUrl.cScheme == "file:") ? true : false);
                        shorteningPath(resUrl, possibleBaseURL, isFile);
                        let basePathStr: string = basePathToString(possibleBaseURL);
                        basePathStr == "" ? basePathStr = strInput : basePathStr += "/" + strInput;
                        resUrl.cPath = new Array<string>();
                        resUrl.analysizeInput(basePathStr);
                        resUrl.flags[6] = true
                    }
                } else if (possibleBaseURL.flags[9]) {
                    resUrl.flags[0] = true
                    result.urlInner = resUrl;
                    return result;
                }
            }
            result.urlInner = resUrl;
            return result;
        }

        static parseBaseUrl(input: string, baseUrl: URL): URL {
            let base: InnerURL = baseUrl.urlInner;
            let result: URL = new URL();
            let resUrl: InnerURL = new InnerURL();
            let strInput: string = input;
            resUrl.parseSingleUrl(strInput);
            if (!resUrl.flags[0]) {
                result.urlInner = resUrl;
                return result;
            }
            if (input.length != 0) {
                if ((input[0] == c'/') && (input[1] == c'/' || (input[1] == c'\\' && base.flags[1]))) {
                    let newInput: string = base.cScheme + input;
                    resUrl.flags[0] = false
                    resUrl.initOnlyInput(newInput);
                    result.urlInner = resUrl;
                    return result;
                }
            }
            if (!base.flags[9]) {
                resUrl.flags[0] = false
                baseIntoUrl(base, resUrl, input.length == 0);
                strInput = resUrl.toolHasBase(input, strInput);
                if (input.length != 0 && input[0] != c'/' && resUrl.cPath.length == 0) {
                    resUrl.cPath = base.cPath;
                    resUrl.flags[6] = base.flags[6];
                }
                if (input.length != 0 && input[0] != c'/' && resUrl.cPath.length != 0) {
                    let isFile: boolean = ((resUrl.cScheme == "file:") ? true : false);
                    shorteningPath(resUrl, base, isFile);
                    let basePathStr: string = basePathToString(base);
                    basePathStr == "" ? basePathStr = strInput : basePathStr += "/" + strInput;
                    resUrl.cPath = new Array<string>();
                    resUrl.analysizeInput(basePathStr);
                    resUrl.flags[6] = true
                }
            } else if (base.flags[9]) {
                resUrl.flags[0] = true
                result.urlInner = resUrl;
                return result;
            }
            result.urlInner = resUrl;
            return result;
        }
    }

    function removeKeyValuePairs(str: string, key: string): string {
        const regexStr = `\\b${key}=[^&]*&?`
        let result = str.replaceAll(regexStr, "");
        if (result.endsWith("&")) {
            result = result.slice(0, -1);
        }
        return result;
    }

    export class URLParams {
        urlClass: NativeURLSearchParams;
        parentUrl: URL | null = null;
        constructor(input?: [string, string][] | Record<string, string> | string | URLParams) {
            let arr = parameterProcess(input);
            this.urlClass = new NativeURLSearchParams();
            this.urlClass.setArray(arr);
        }

        append(params1: string, params2: string): void {
            params1 = fixUSVstring(params1);
            params2 = fixUSVstring(params2);
            this.urlClass.append(params1, params2);
            if (this.parentUrl != null) {
                this.parentUrl!.urlInner.setSearch(this.toString());
                this.parentUrl!.cSearch = this.parentUrl!.urlInner.getSearch();
                this.parentUrl!.setHref();
            }
        }
        set(setName: string, setValues: string): void {
            setName = fixUSVstring(setName);
            setValues = fixUSVstring(setValues);
            this.urlClass.set(setName, setValues);
            if (this.parentUrl != null) {
                this.parentUrl!.urlInner.setSearch(this.toString());
                this.parentUrl!.cSearch = this.parentUrl!.urlInner.getSearch();
                this.parentUrl!.setHref();
            }
        }
        sort(): void {
            this.urlClass.sort();
            if (this.parentUrl != null) {
                this.parentUrl!.urlInner.setSearch(this.toString());
                this.parentUrl!.cSearch = this.parentUrl!.urlInner.getSearch();
                this.parentUrl!.setHref();
            }
        }
        has(hasname: string): boolean {
            hasname = fixUSVstring(hasname);
            return this.urlClass.isHas(hasname);
        }
        toString(): string {
            return this.urlClass.toString();
        }
        keys(): IterableIterator<string> {
            return this.urlClass.iterByKeys().$_iterator();
        }

        values(): IterableIterator<string> {
            return this.urlClass.iterByValues().$_iterator();
        }

        getAll(getAllname: string): Array<string> {
            getAllname = fixUSVstring(getAllname);
            return this.urlClass.getAll(getAllname);
        }

        get(getname: string): string | undefined {
            getname = fixUSVstring(getname);
            return this.urlClass.get(getname);
        }

        entries(): IterableIterator<[string,string]> {
            return this.urlClass.entries().$_iterator();
        }

        delete(deleteName: string): void {
            deleteName = fixUSVstring(deleteName);
            this.urlClass.delete(deleteName);
            if (this.parentUrl != null) {
                let searchStr: string = removeKeyValuePairs(this.parentUrl!.urlInner.getSearch(), deleteName);
                this.parentUrl!.urlInner.setSearch(searchStr);
                this.parentUrl!.cSearch = searchStr;
                this.parentUrl!.setHref();
            }
        }

        forEach(
            objfun: (value: string, key: string, param: URLParams) => void
        ) {
            let array = this.urlClass.getArray();
            if (array.length == 0) {
                return;
            }
            let size = array.length - 1;
            for (let i = 0; i < size; i += 2) { // 2:Searching for the number and number of keys and values 2
                let key = array[i];
                let value = array[i + 1];
                objfun(value, key, this);
            }
        }

        $_iterator(): IterableIterator<[string, string]> {
            return this.urlClass.entries().$_iterator();
        }

        updateParams(input: string): void {
            let arr = parameterProcess(input);
            this.urlClass.setArray(arr);
        }
    }

    class NativeURLSearchParams {
        private searchParams = new Array<string>();

        constructor() { }

        get(key: string): string | undefined {
            let len = this.searchParams.length;
            for (let i = 0; i < len; i += 2) {
                if (key == this.searchParams[i]) {
                    return this.searchParams[i + 1];
                }
            }
            return undefined;
        }

        getAll(key: string): Array<string> {
            let result = new Array<string>();
            let len = this.searchParams.length;
            for (let i = 0; i < len; i += 2) {
                if (key == this.searchParams[i]) {
                    result.push(this.searchParams[i + 1]);
                }
            }
            return result;
        }

        append(key: string, value: string) {
            this.searchParams.push(key);
            this.searchParams.push(value);
        }

        delete(name: string) {
            for (let i = this.searchParams.length - 2; i >= 0; i -= 2) {
                if (this.searchParams[i] == name) {
                    this.searchParams.splice(i, 2);
                }
            }
        }

        entries(): Array<[string,string]> {
            let result = new Array<[string, string]>();
            for (let i = 0; i < this.searchParams.length; i += 2) {
                let arr: [string, string] = [this.searchParams[i], this.searchParams[i + 1]]; 
                result.push(arr);
            }
            return result;

        }

        isHas(str: string): boolean {//key
            let len = this.searchParams.length;
            for (let i = 0; i < len; i += 2) {
                if (this.searchParams[i] == str) {
                    return true;
                }
            }
            return false;

        }

        set(key: string, value: string) {
            let len = this.searchParams.length;
            let flag = false;
            for (let i = 0; i < len;) {
                if (this.searchParams[i] == key) {
                    if (!flag) {
                        this.searchParams[i + 1] = value;
                        flag = true;
                        i += 2;

                    } else {
                        this.searchParams.splice(i, 2);
                    }
                } else {
                    i += 2;
                }
            }
            if (!flag) {
                this.append(key, value);
            }
        }

        sort() {
            let len = this.searchParams.length;
            if (len <= 2 || (len % 2 != 0)) { // 2: Iterate over key-value pairs
                return;
            }
            let i = 0;
            for (; i < len - 2; i += 2) { // 2:Iterate over key-value pairs
                let j = i + 2; // 2:Iterate over key-value pairs
                for (; j < len; j += 2) { // 2:Iterate over key-value pairs
                    let tmp: boolean = (this.searchParams[i] > this.searchParams[j]);
                    if (tmp) {
                        let curKey = this.searchParams[i];
                        let curVal = this.searchParams[i + 1];
                        this.searchParams[i] = this.searchParams[j];
                        this.searchParams[i + 1] = this.searchParams[j + 1];
                        this.searchParams[j] = curKey;
                        this.searchParams[j + 1] = curVal;
                    }
                }
            }
        }

        iterByKeys(): Array<string> {
            let res = new Array<string>();
            let len = this.searchParams.length;
            for (let i = 0; i < len; i += 2) {
                res.push(this.searchParams[i]);
            }
            return res;
        }

        getArray(): Array<string> {
            return this.searchParams;
        }

        setArray(arr: Array<string>) {
            this.searchParams = arr;
        }

        iterByValues(): Array<string> {
            let res = new Array<string>();
            let len = this.searchParams.length;
            for (let i = 1; i < len; i += 2) {
                res.push(this.searchParams[i]);
            }
            return res;
        }

        toString(): string {
            let resultArray = new Array<string>();
            let array = this.getArray();
            let arrayLen: number = array.length;
            let key: string = '';
            let value: string = '';
            const regStr='%20';
            for (let pos: number = 0; pos < arrayLen; pos += 2) { // 2:Even subscripts exist as key values
                key = encodePercentEncoding(array[pos], Helper.PARAMS_TOSTRING_SIGN_CHARS).replaceAll(regStr, '+');
                value = encodePercentEncoding(array[pos + 1], Helper.PARAMS_TOSTRING_SIGN_CHARS).replaceAll(regStr, '+');
                resultArray.push(`${pos > 0 ? '&' : ''}${key}=${value}`);
            }
            return resultArray.join('');
        }
    }

    let seachParamsArr = new Array<string>();

    export function parameterProcess(input?: [string, string][] | Record<string, string> | string | URLParams): Array<string> {
        if (input == undefined || input == null || input == '') {
            seachParamsArr = new Array<string>();
            return seachParamsArr;
        } else if (typeof input == 'string') {
            return parseParamFromString(fixUSVstring(input as string));
        } else if (input instanceof URLParams) {
            return input.urlClass.getArray();
        } else if (input instanceof [string, string][]) {
            return parseParamFromArray(input as [string, string][]);
        } else if (input instanceof Record) {
            return parseParamFromRecord(input as Record<string, string>)
        } else {
            throw createBusinessError(SyntaxErrorCodeId,'Syntax Error. Invalid input type.');
        }
    }

    function parseParamFromArray(input: [string, string][]): Array<string> {
        let result = new Array<string>();
        if (input.length == 0) {
            return result;
        } else if (input[0][0] == undefined || input[0][1] == undefined) {
            throw createBusinessError(SyntaxErrorCodeId,'Parameter error. The type of input must be string[][].');
        }

        for (let it of input) {
            result.push(fixUSVstring(it[0]));
            result.push(fixUSVstring(it[1]));
        }
        return result;
    }

    function parseParamFromRecord(input: Record<string, string>): Array<string> {
        let result = new Array<string>();
        for (let k of input.keys()) {
            result.push(fixUSVstring(k));
            result.push(fixUSVstring(input.get(k)!));
        }
        return result;
    }

    function parseParamFromString(input: string): Array<string> {
        if (input[0] == c'?') {
            input = input.slice(1);
        }
        let strVal = input.replaceAll('+', ' ');
        seachParamsArr = stringParmas(strVal);
        return seachParamsArr.map((item: string) => item = decodeStringParmas(item));
    }

    function utf8Decode(uint8Array: Uint8Array) {
        let result: string = '';
        let i = 0;
        const UNICODE_0x80 = 0x80;
        const UNICODE_0x800 = 0x800;
        const UNICODE_0xD800 = 0xD800;
        const UNICODE_0xE000 = 0xE000;
        const UNICODE_0xC0 = 0xC0;
        const UNICODE_0x3F = 0x3F;
        const UNICODE_0xF0 = 0xF0;
        const UNICODE_0xE0 = 0xE0;
        const UNICODE_0x07 = 0x07;
        const UNICODE_0x0F = 0x0F;
        const UNICODE_0XFFFD = 0XFFFD;
        const UNICODE_0x1F = 0x1F;
        while (i < uint8Array.length) {
            const byte1 = uint8Array[i];
            if ((byte1 & UNICODE_0x80) === 0) {
                //  (0xxxxxxx)
                result += String.fromCharCode(byte1);
                i++;
            } else if ((byte1 & UNICODE_0xE0) === UNICODE_0xC0) {
                if (i + 1 >= uint8Array.length) {
                    result += String.fromCharCode(UNICODE_0XFFFD);
                    i++;
                    continue;
                }
                //  (110xxxxx 10xxxxxx)
                const byte2 = uint8Array[i + 1];
                const codePoint = ((byte1 & UNICODE_0x1F) << 6) | (byte2 & UNICODE_0x3F);
                result += String.fromCharCode(codePoint);
                i += 2;
            } else if ((byte1 & UNICODE_0xF0) === UNICODE_0xE0) {
                if (i + 2 >= uint8Array.length) {
                    result += String.fromCharCode(UNICODE_0XFFFD);
                    i++;
                    continue;
                }
                //  (1110xxxx 10xxxxxx 10xxxxxx)
                const byte2 = uint8Array[i + 1];
                const byte3 = uint8Array[i + 2];
                const codePoint = ((byte1 & UNICODE_0x0F) << 12) | ((byte2 & UNICODE_0x3F) << 6) | (byte3 & UNICODE_0x3F);
                result += String.fromCharCode(codePoint);
                i += 3;
            } else if ((byte1 & 0xF8) === UNICODE_0xF0) {
                // (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
                if (i + 3 >= uint8Array.length) {
                    result += String.fromCharCode(UNICODE_0XFFFD);
                    i++;
                    continue;
                }

                const byte2 = uint8Array[i + 1];
                const byte3 = uint8Array[i + 2];
                const byte4 = uint8Array[i + 3];
                const codePoint = ((byte1 & UNICODE_0x07) << 18) | ((byte2 & UNICODE_0x3F) << 12) | ((byte3 & UNICODE_0x3F) << 6) | (byte4 & UNICODE_0x3F);
                result += String.fromCodePoint(codePoint);
                i += 4;
            } else {
                result += String.fromCharCode(UNICODE_0XFFFD);
                i++;
            }
        }
        return result;
    }

    function unescapeBuffer(s: string): Uint8Array {
        const outBuffer: Uint8Array = new Uint8Array(s.length);
        let index = 0;
        let outIndex = 0;
        let currentChar: number = 0;
        let nextChar = 0;
        let hexHigh: number = 0;
        let hexLow: number = 0;
        const maxLength = s.length - 2;
        // Flag to know if some hex chars have been decoded
        let hasHex = false;
        let code_plus = 43;
        let code_percent = 37;
        let code_space = 32;
        while (index < s.length) {
            currentChar = s.charCodeAt(index);
            if (currentChar == code_plus) {
                outBuffer[outIndex++] = code_space;
                index++;
                continue;
            }
            if (currentChar == code_percent && index < maxLength) {
                currentChar = s.charCodeAt(++index);
                hexHigh = Helper.unhexTable[currentChar];
                if (hexHigh < 0) {
                    outBuffer[outIndex++] = code_percent;
                    continue;
                } else {
                    nextChar = s.charCodeAt(++index);
                    hexLow = Helper.unhexTable[nextChar];
                    if (!(hexLow >= 0)) {
                        outBuffer[outIndex++] = code_percent;
                        index--;
                    } else {
                        hasHex = true;
                        currentChar = hexHigh * 16 + hexLow;
                    }
                }
            }
            outBuffer[outIndex++] = currentChar;
            index++;
        }
        return hasHex ? outBuffer.slice(0, outIndex) : outBuffer;
    }

    function decodeStringParmas(input: string): string {
        let strVal = '';
        try {
            strVal = decodeURIComponent(input);
        } catch (e) {
            strVal = utf8Decode(unescapeBuffer(input));
        }
        return strVal;
    }

    function decodeSafelyOut(input: string): string {
        let decodedString: string = '';
        let decodedTemp: string = '';
        let index: number = 0;
        while (index < input.length) {
            if (input[index] == c'%' && new RegExp('[0-9A-Fa-f]{2}').test(input.slice(index + 1, index + 3))) {
                const encodedChar = input.slice(index, index + 3);
                try {
                    decodedString += decodeURIComponent(decodedTemp + encodedChar);
                    decodedTemp = '';
                } catch (e) {
                    decodedTemp += encodedChar;
                }
                index += 3;
                continue;
            }
            if (decodedTemp == '') {
                decodedString += input[index];
            } else {
                decodedString += decodedTemp;
                decodedString += input[index];
                decodedTemp = '';
            }
            index++;
        }
        return decodedTemp == '' ? decodedString : decodedString += decodedTemp;
    }

    function stringParmas(stringParm: string): Array<string> {
        let seachParasVec = new Array<string>();
        let strStartPos = 0;
        let strLastPos = 0;
        let isHasSpace = false;
        let buf = "";
        let i = 0;
        for (; i < stringParm.length; i++) {
            let code = stringParm[i];
            switch (code) {
                case c'&':
                    {
                        if (strStartPos == i) {
                            strLastPos = i + 1;
                            strStartPos = i + 1;
                            continue;
                        }
                        if (strLastPos < i) {
                            buf += stringParm.substr(strLastPos, i - strLastPos);
                        }
                        seachParasVec.push(buf);
                        if (!isHasSpace) {
                            seachParasVec.push("");
                        }
                        isHasSpace = false;
                        buf = "";
                        strLastPos = i + 1;
                        strStartPos = i + 1;
                        break;

                    }
                case c'=':
                    {
                        if (isHasSpace) {
                            break;
                        }
                        if (strLastPos < i) {
                            buf += stringParm.substr(strLastPos, i - strLastPos);
                        }
                        seachParasVec.push(buf);
                        buf = "";
                        strLastPos = i + 1;
                        isHasSpace = true;
                        break;
                    }
                default: break;
            }
        }
        if (strStartPos == i) {
            return seachParasVec;
        }
        if (strLastPos < i) {
            buf += stringParm.substr(strLastPos, i - strLastPos);
        }
        seachParasVec.push(buf);
        if (!isHasSpace) {
            seachParasVec.push("");
        }

        return seachParasVec;
    }

    const unpairedSurrogateRe: RegExp =
        new RegExp("(?:[^\uD800-\uDBFF]|^)[\uDC00-\uDFFF]|[\uD800-\uDBFF](?![\uDC00-\uDFFF])");

    function fixUSVstring(str: string) {
        return str.toWellFormed();
    }

    function join(arr: Array<string>, s: string): string {
        let result = '';
        for (let i = 0; i < arr.length; i++) {
            result += arr[i];
            if (i != arr.length - 1) {
                result += s;
            }
        }
        return result;
    }

    function encodePercentEncoding(str: string, noEscapeTable: Int8Array) {
        const len = str.length;
        if (len == 0)
            return '';

        let outVal = '';
        let lastPos = 0;
        let i = 0;
        const UNICODE_0x80 = 0x80;
        const UNICODE_0x800 = 0x800;
        const UNICODE_0xD800 = 0xD800;
        const UNICODE_0xE000 = 0xE000;
        const UNICODE_0xC0 = 0xC0;
        const UNICODE_0x3F = 0x3F;
        const UNICODE_0xF0 = 0xF0;
        const UNICODE_0xE0 = 0xE0;
        const UNICODE_0x3FF = 0x3FF;
        const UNICODE_0x10000 = 0x10000;
        outer:
        for (; i < len; i++) {
            let c = str.charCodeAt(i as number);

            // ASCII
            while (c < UNICODE_0x80) {
                if (noEscapeTable[c] !== 1) {
                    if (lastPos < i)
                        outVal += str.slice(lastPos, i);
                    lastPos = i + 1;
                    outVal += Helper.hexTable[c];
                }

                if (++i == len)
                    break outer;

                c = str.charCodeAt(i);
            }

            if (lastPos < i)
                outVal += str.slice(lastPos, i);

            // Multi-byte characters ...
            if (c < UNICODE_0x800) {
                lastPos = i + 1;
                outVal += Helper.hexTable[UNICODE_0xC0 | (c >> 6)] +
                    Helper.hexTable[UNICODE_0x80 | (c & UNICODE_0x3F)];
                continue;
            }
            if (c < UNICODE_0xD800 || c >= UNICODE_0xE000) {
                lastPos = i + 1;
                outVal += Helper.hexTable[UNICODE_0xE0 | (c >> 12)] +
                    Helper.hexTable[UNICODE_0x80 | ((c >> 6) & UNICODE_0x3F)] +
                    Helper.hexTable[UNICODE_0x80 | (c & UNICODE_0x3F)];
                continue;
            }
            // Surrogate pair
            ++i;

            const c2 = str.charCodeAt(i) & UNICODE_0x3FF;

            lastPos = i + 1;
            c = UNICODE_0x10000 + (((c & UNICODE_0x3FF) << 10) | c2);
            outVal += Helper.hexTable[UNICODE_0xF0 | (c >> 18)] +
                Helper.hexTable[UNICODE_0x80 | ((c >> 12) & UNICODE_0x3F)] +
                Helper.hexTable[UNICODE_0x80 | ((c >> 6) & UNICODE_0x3F)] +
                Helper.hexTable[UNICODE_0x80 | (c & UNICODE_0x3F)];
        }
        if (lastPos == 0) {
            return str;
        }
        if (lastPos < len) {
            return outVal + str.slice(lastPos);
        }
        return outVal;
    }
}
