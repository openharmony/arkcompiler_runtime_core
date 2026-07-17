/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class CharHolder {
    public ch: char = c'\0'
}

class MultiCharHolder {
    public newline: char = c'\0'
    public tab: char = c'\0'
    public quote: char = c'\0'
    public backslash: char = c'\0'
    public normal: char = c'\0'
}

class ObjectCharHolder {
    public value: Object | undefined = undefined
}

class ObjectCharMultiHolder {
    public a: Object | undefined = undefined
    public b: Object | undefined = undefined
    public c: Object | undefined = undefined
}

function jsonStringifyCharNewline(): void {
    arktest.assertEQ(JSON.stringify(c'\n'), '"\\n"')
}

function jsonStringifyCharTab(): void {
    arktest.assertEQ(JSON.stringify(c'\t'), '"\\t"')
}

function jsonStringifyCharCarriageReturn(): void {
    arktest.assertEQ(JSON.stringify(c'\r'), '"\\r"')
}

function jsonStringifyCharQuote(): void {
    arktest.assertEQ(JSON.stringify(c'"'), '"\\""')
}

function jsonStringifyCharBackslash(): void {
    arktest.assertEQ(JSON.stringify(c'\\'), '"\\\\"')
}

function jsonStringifyCharBackspace(): void {
    arktest.assertEQ(JSON.stringify(c'\b'), '"\\b"')
}

function jsonStringifyCharFormFeed(): void {
    arktest.assertEQ(JSON.stringify(c'\f'), '"\\f"')
}

function jsonStringifyCharNormal(): void {
    arktest.assertEQ(JSON.stringify(c'A'), '"A"')
    arktest.assertEQ(JSON.stringify(c'0'), '"0"')
    arktest.assertEQ(JSON.stringify(c' '), '" "')
}

function jsonStringifyCharControlCharacters(): void {
    arktest.assertEQ(JSON.stringify(c'\0'), '"\\u0000"')
    arktest.assertEQ(JSON.stringify(c'\u0001'), '"\\u0001"')
    arktest.assertEQ(JSON.stringify(c'\u001f'), '"\\u001f"')
}

function jsonStringifyCharDel(): void {
    arktest.assertEQ(JSON.stringify(c'\u007f'), '"\\u007f"')
}

function jsonStringifyFixedArrayCharEscaping(): void {
    let arr: FixedArray<char> = new FixedArray<char>(4)
    arr[0] = c'\n'
    arr[1] = c'\t'
    arr[2] = c'"'
    arr[3] = c'A'
    arktest.assertEQ(JSON.stringify(arr), '["\\n","\\t","\\"","A"]')
}

function jsonStringifyFixedArrayCharBackslash(): void {
    let arr: FixedArray<char> = new FixedArray<char>(2)
    arr[0] = c'\\'
    arr[1] = c'B'
    arktest.assertEQ(JSON.stringify(arr), '["\\\\","B"]')
}

function jsonStringifyFixedArrayCharEmpty(): void {
    let arr: FixedArray<char> = new FixedArray<char>(0)
    arktest.assertEQ(JSON.stringify(arr), '[]')
}

function jsonStringifyFixedArrayCharSingle(): void {
    let arr: FixedArray<char> = new FixedArray<char>(1)
    arr[0] = c'X'
    arktest.assertEQ(JSON.stringify(arr), '["X"]')
}

function jsonStringifyFixedArrayCharControlCharacters(): void {
    let arr: FixedArray<char> = new FixedArray<char>(3)
    arr[0] = c'\0'
    arr[1] = c'\r'
    arr[2] = c'\u001f'
    arktest.assertEQ(JSON.stringify(arr), '["\\u0000","\\r","\\u001f"]')
}

function jsonStringifyFixedArrayCharDel(): void {
    let arr: FixedArray<char> = new FixedArray<char>(2)
    arr[0] = c'\u007f'
    arr[1] = c'A'
    arktest.assertEQ(JSON.stringify(arr), '["\\u007f","A"]')
}

function jsonStringifyValueArrayCharEscaping(): void {
    let arr: ValueArray<char> = new ValueArray<char>(3)
    arr[0] = c'\n'
    arr[1] = c'"'
    arr[2] = c'A'
    arktest.assertEQ(JSON.stringify(arr), '["\\n","\\"","A"]')
}

function jsonStringifyValueArrayCharEmpty(): void {
    let arr: ValueArray<char> = new ValueArray<char>(0)
    arktest.assertEQ(JSON.stringify(arr), '[]')
}

function jsonStringifyClassWithCharField(): void {
    let obj = new CharHolder()
    obj.ch = c'\n'
    arktest.assertEQ(JSON.stringify(obj), '{"ch":"\\n"}')
}

function jsonStringifyClassWithCharFieldQuote(): void {
    let obj = new CharHolder()
    obj.ch = c'"'
    arktest.assertEQ(JSON.stringify(obj), '{"ch":"\\""}')
}

function jsonStringifyClassWithCharFieldBackslash(): void {
    let obj = new CharHolder()
    obj.ch = c'\\'
    arktest.assertEQ(JSON.stringify(obj), '{"ch":"\\\\"}')
}

function jsonStringifyClassWithCharFieldNormal(): void {
    let obj = new CharHolder()
    obj.ch = c'A'
    arktest.assertEQ(JSON.stringify(obj), '{"ch":"A"}')
}

function jsonStringifyClassWithMultipleCharFields(): void {
    let obj = new MultiCharHolder()
    obj.newline = c'\n'
    obj.tab = c'\t'
    obj.quote = c'"'
    obj.backslash = c'\\'
    obj.normal = c'X'
    arktest.assertEQ(JSON.stringify(obj), '{"newline":"\\n","tab":"\\t","quote":"\\"","backslash":"\\\\","normal":"X"}')
}

function jsonStringifyCharRoundTrip(): void {
    let chars: FixedArray<char> = [c'\n', c'\t', c'\r', c'"', c'\\', c'\b', c'\f', c'A', c' ']
    for (let i = 0; i < chars.length; i++) {
        let json = JSON.stringify(chars[i])
        let parsed = JSON.parse<String>(json, Class.from<String>())
        arktest.assertEQ(parsed, new String(chars[i]))
    }
}

function jsonStringifyObjectFieldCharNewline(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'\n')
    arktest.assertEQ(JSON.stringify(obj), '{"value":"\\n"}')
}

function jsonStringifyObjectFieldCharTab(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'\t')
    arktest.assertEQ(JSON.stringify(obj), '{"value":"\\t"}')
}

function jsonStringifyObjectFieldCharQuote(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'"')
    arktest.assertEQ(JSON.stringify(obj), '{"value":"\\""}')
}

function jsonStringifyObjectFieldCharBackslash(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'\\')
    arktest.assertEQ(JSON.stringify(obj), '{"value":"\\\\"}')
}

function jsonStringifyObjectFieldCharBackspace(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'\b')
    arktest.assertEQ(JSON.stringify(obj), '{"value":"\\b"}')
}

function jsonStringifyObjectFieldCharNormal(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'A')
    arktest.assertEQ(JSON.stringify(obj), '{"value":"A"}')
}

function jsonStringifyObjectFieldCharControlChar(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'\0')
    arktest.assertEQ(JSON.stringify(obj), '{"value":"\\u0000"}')
}

function jsonStringifyObjectFieldCharDel(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'\u007f')
    arktest.assertEQ(JSON.stringify(obj), '{"value":"\\u007f"}')
}

function jsonStringifyObjectFieldMultipleChars(): void {
    let obj = new ObjectCharMultiHolder()
    obj.a = new Char(c'\n')
    obj.b = new Char(c'"')
    obj.c = new Char(c'X')
    arktest.assertEQ(JSON.stringify(obj), '{"a":"\\n","b":"\\"","c":"X"}')
}

function jsonStringifyObjectFieldCharWithReplacer(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'\n')
    let result = JSON.stringify(obj, (key: string, val: Any): Any => val, 2)
    arktest.assertTrue(result.includes('"value": "\\n"'))
}

function jsonStringifyCharHighSurrogate(): void {
    arktest.assertEQ(JSON.stringify(c'\uD800'), '"\\ud800"')
    arktest.assertEQ(JSON.stringify(c'\uDBFF'), '"\\udbff"')
}

function jsonStringifyCharLowSurrogate(): void {
    arktest.assertEQ(JSON.stringify(c'\uDC00'), '"\\udc00"')
    arktest.assertEQ(JSON.stringify(c'\uDFFF'), '"\\udfff"')
}

function jsonStringifyFixedArrayCharSurrogates(): void {
    let arr: FixedArray<char> = new FixedArray<char>(4)
    arr[0] = c'\uD800'
    arr[1] = c'\uDBFF'
    arr[2] = c'\uDC00'
    arr[3] = c'\uDFFF'
    arktest.assertEQ(JSON.stringify(arr), '["\\ud800","\\udbff","\\udc00","\\udfff"]')
}

function jsonStringifyValueArrayCharSurrogates(): void {
    let arr: ValueArray<char> = new ValueArray<char>(4)
    arr[0] = c'\uD800'
    arr[1] = c'\uDBFF'
    arr[2] = c'\uDC00'
    arr[3] = c'\uDFFF'
    arktest.assertEQ(JSON.stringify(arr), '["\\ud800","\\udbff","\\udc00","\\udfff"]')
}

function jsonStringifyClassWithCharFieldHighSurrogate(): void {
    let obj = new CharHolder()
    obj.ch = c'\uD800'
    arktest.assertEQ(JSON.stringify(obj), '{"ch":"\\ud800"}')
}

function jsonStringifyClassWithCharFieldLowSurrogate(): void {
    let obj = new CharHolder()
    obj.ch = c'\uDC00'
    arktest.assertEQ(JSON.stringify(obj), '{"ch":"\\udc00"}')
}

function jsonStringifyObjectFieldCharHighSurrogate(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'\uD800')
    arktest.assertEQ(JSON.stringify(obj), '{"value":"\\ud800"}')
}

function jsonStringifyObjectFieldCharLowSurrogate(): void {
    let obj = new ObjectCharHolder()
    obj.value = new Char(c'\uDC00')
    arktest.assertEQ(JSON.stringify(obj), '{"value":"\\udc00"}')
}

function jsonStringifyCharSurrogateRoundTrip(): void {
    let surrogates: FixedArray<char> = [c'\uD800', c'\uDBFF', c'\uDC00', c'\uDFFF']
    for (let i = 0; i < surrogates.length; i++) {
        let json = JSON.stringify(surrogates[i])
        let parsed = JSON.parse<String>(json, Class.from<String>())
        arktest.assertEQ(parsed, new String(surrogates[i]))
    }
}

function jsonStringifyValueArrayCharControlChars(): void {
    let arr: ValueArray<char> = new ValueArray<char>(4)
    arr[0] = c'\0'
    arr[1] = c'\r'
    arr[2] = c'\u001f'
    arr[3] = c'A'
    arktest.assertEQ(JSON.stringify(arr), '["\\u0000","\\r","\\u001f","A"]')
}

function jsonStringifyValueArrayCharDel(): void {
    let arr: ValueArray<char> = new ValueArray<char>(2)
    arr[0] = c'\u007f'
    arr[1] = c'A'
    arktest.assertEQ(JSON.stringify(arr), '["\\u007f","A"]')
}

function jsonStringifyValueArrayCharBackslash(): void {
    let arr: ValueArray<char> = new ValueArray<char>(2)
    arr[0] = c'\\'
    arr[1] = c'B'
    arktest.assertEQ(JSON.stringify(arr), '["\\\\","B"]')
}

function jsonStringifyCharBoundaryBeforeSurrogates(): void {
    arktest.assertEQ(JSON.stringify(c'\uD7FF'), '"\uD7FF"')
}

function jsonStringifyCharBoundaryAfterSurrogates(): void {
    arktest.assertEQ(JSON.stringify(c'\uE000'), '"\uE000"')
}

function jsonStringifyCharNonAsciiBmp(): void {
    arktest.assertEQ(JSON.stringify(c'\u00E9'), '"\u00E9"')
    arktest.assertEQ(JSON.stringify(c'\u4E2D'), '"\u4E2D"')
}

function jsonStringifyFixedArrayCharSurrogatesAndNormal(): void {
    let arr: FixedArray<char> = new FixedArray<char>(3)
    arr[0] = c'A'
    arr[1] = c'\uD800'
    arr[2] = c'B'
    arktest.assertEQ(JSON.stringify(arr), '["A","\\ud800","B"]')
}

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.stringify char escaping')
    suite.addTest('JSON.stringify char newline', jsonStringifyCharNewline)
    suite.addTest('JSON.stringify char tab', jsonStringifyCharTab)
    suite.addTest('JSON.stringify char carriage return', jsonStringifyCharCarriageReturn)
    suite.addTest('JSON.stringify char quote', jsonStringifyCharQuote)
    suite.addTest('JSON.stringify char backslash', jsonStringifyCharBackslash)
    suite.addTest('JSON.stringify char backspace', jsonStringifyCharBackspace)
    suite.addTest('JSON.stringify char form feed', jsonStringifyCharFormFeed)
    suite.addTest('JSON.stringify char normal', jsonStringifyCharNormal)
    suite.addTest('JSON.stringify char control characters', jsonStringifyCharControlCharacters)
    suite.addTest('JSON.stringify char DEL', jsonStringifyCharDel)
    suite.addTest('JSON.stringify FixedArray<char> escaping', jsonStringifyFixedArrayCharEscaping)
    suite.addTest('JSON.stringify FixedArray<char> backslash', jsonStringifyFixedArrayCharBackslash)
    suite.addTest('JSON.stringify FixedArray<char> empty', jsonStringifyFixedArrayCharEmpty)
    suite.addTest('JSON.stringify FixedArray<char> single', jsonStringifyFixedArrayCharSingle)
    suite.addTest('JSON.stringify FixedArray<char> control characters', jsonStringifyFixedArrayCharControlCharacters)
    suite.addTest('JSON.stringify FixedArray<char> DEL', jsonStringifyFixedArrayCharDel)
    suite.addTest('JSON.stringify ValueArray<char> escaping', jsonStringifyValueArrayCharEscaping)
    suite.addTest('JSON.stringify ValueArray<char> empty', jsonStringifyValueArrayCharEmpty)
    suite.addTest('JSON.stringify class with char field newline', jsonStringifyClassWithCharField)
    suite.addTest('JSON.stringify class with char field quote', jsonStringifyClassWithCharFieldQuote)
    suite.addTest('JSON.stringify class with char field backslash', jsonStringifyClassWithCharFieldBackslash)
    suite.addTest('JSON.stringify class with char field normal', jsonStringifyClassWithCharFieldNormal)
    suite.addTest('JSON.stringify class with multiple char fields', jsonStringifyClassWithMultipleCharFields)
    suite.addTest('JSON.stringify char round trip', jsonStringifyCharRoundTrip)
    suite.addTest('JSON.stringify Object field Char newline', jsonStringifyObjectFieldCharNewline)
    suite.addTest('JSON.stringify Object field Char tab', jsonStringifyObjectFieldCharTab)
    suite.addTest('JSON.stringify Object field Char quote', jsonStringifyObjectFieldCharQuote)
    suite.addTest('JSON.stringify Object field Char backslash', jsonStringifyObjectFieldCharBackslash)
    suite.addTest('JSON.stringify Object field Char backspace', jsonStringifyObjectFieldCharBackspace)
    suite.addTest('JSON.stringify Object field Char normal', jsonStringifyObjectFieldCharNormal)
    suite.addTest('JSON.stringify Object field Char control char', jsonStringifyObjectFieldCharControlChar)
    suite.addTest('JSON.stringify Object field Char DEL', jsonStringifyObjectFieldCharDel)
    suite.addTest('JSON.stringify Object field multiple Chars', jsonStringifyObjectFieldMultipleChars)
    suite.addTest('JSON.stringify Object field Char with replacer', jsonStringifyObjectFieldCharWithReplacer)
    suite.addTest('JSON.stringify char high surrogate', jsonStringifyCharHighSurrogate)
    suite.addTest('JSON.stringify char low surrogate', jsonStringifyCharLowSurrogate)
    suite.addTest('JSON.stringify FixedArray<char> surrogates', jsonStringifyFixedArrayCharSurrogates)
    suite.addTest('JSON.stringify ValueArray<char> surrogates', jsonStringifyValueArrayCharSurrogates)
    suite.addTest('JSON.stringify class with char field high surrogate', jsonStringifyClassWithCharFieldHighSurrogate)
    suite.addTest('JSON.stringify class with char field low surrogate', jsonStringifyClassWithCharFieldLowSurrogate)
    suite.addTest('JSON.stringify Object field Char high surrogate', jsonStringifyObjectFieldCharHighSurrogate)
    suite.addTest('JSON.stringify Object field Char low surrogate', jsonStringifyObjectFieldCharLowSurrogate)
    suite.addTest('JSON.stringify char surrogate round trip', jsonStringifyCharSurrogateRoundTrip)
    suite.addTest('JSON.stringify ValueArray<char> control chars', jsonStringifyValueArrayCharControlChars)
    suite.addTest('JSON.stringify ValueArray<char> DEL', jsonStringifyValueArrayCharDel)
    suite.addTest('JSON.stringify ValueArray<char> backslash', jsonStringifyValueArrayCharBackslash)
    suite.addTest('JSON.stringify char boundary before surrogates (D7FF)', jsonStringifyCharBoundaryBeforeSurrogates)
    suite.addTest('JSON.stringify char boundary after surrogates (E000)', jsonStringifyCharBoundaryAfterSurrogates)
    suite.addTest('JSON.stringify char non-ASCII BMP', jsonStringifyCharNonAsciiBmp)
    suite.addTest('JSON.stringify FixedArray<char> surrogates and normal', jsonStringifyFixedArrayCharSurrogatesAndNormal)
    return suite.run()
}
