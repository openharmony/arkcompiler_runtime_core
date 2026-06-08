/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON stringify and parse Unicode reviver tests')
    suite.addTest('emoji string round-trip', unicodeEmojiRoundtrip)
    suite.addTest('reviver replaces container', reviverReplaceContainer)
    suite.addTest('accented char round-trip', unicodeAccentRoundtrip)
    suite.addTest('reviver sums fields', reviverComputeSum)
    suite.addTest('reviver truncates strings', reviverTruncateStr)
    suite.addTest('nested string with newlines', nestedNewlineRoundtrip)
    suite.addTest('reviver renames property', reviverRenameKey)
    suite.addTest('jsonElement string to num', jsonElemStrToNum)
    suite.addTest('special chars nested array', specialCharNestedArr)
    suite.addTest('multilingual text round-trip', multiLangRoundtrip)
    suite.addTest('reviver truncates array', reviverTruncateArr)
    suite.addTest('reviver computes product', reviverComputeProduct)
    suite.addTest('zero-width joiner round-trip', zeroWidthJoinerRoundtrip)
    suite.addTest('control char round-trip', controlCharRoundtrip)
    suite.addTest('reviver converts date string', reviverDateConvert)
    suite.addTest('family emoji round-trip', familyEmojiRoundtrip)
    suite.addTest('reviver computes cube roots', reviverCubeRoot)
    suite.addTest('CRLF newline round-trip', crlfNewlineRoundtrip)
    suite.addTest('reviver encrypts/decrypts email', reviverReverseEmail)
    suite.addTest('deep special char round-trip', deepSpecialKeyRoundtrip)
    suite.addTest('reviver removes empty arrays', reviverRemoveEmptyArr)
    suite.addTest('multilingual punct round-trip', multiPunctRoundtrip)
    suite.addTest('reviver adds matrix metadata', reviverMatrixMeta)
    return suite.run()
}

class ReplacedContainerValue {
    value: string = '';
}

class RootNestedReplaceObj {
    container: ReplacedContainerValue = { value: '' };
}

class SumSourceFields {
    a: number;
    b: number;
}

class SumResultFields extends SumSourceFields {
    sum: number;
}

class StrTruncateObj {
    shortTest: string = '';
    longTest: string = '';
}

class NewlineLevelFields {
    level2: string = '';
}

class NestedNewlineObj {
    level1: NewlineLevelFields = { level2: '' };
}

class RenamePropObj {
    newKey: string = '';
}

class TempValueFields {
    temp: string = '';
}

type SpecialCharArr = (string | string[])[];

class MultiLangObj {
    en: string = '';
    zh: string = '';
    ja: string = '';
}

class ProductCompare {
    numbers: number[] = [];
    product: number;
}

class TimestampedLogEntry {
    event: string = '';
    time: string = '';
}

class LogListObject {
    logs: TimestampedLogEntry[] = [];
}

class EncryptedObj {
    username: string = '';
    email: string = '';
}

class SpecialCharLeafFields {
    key232: number;
}

class SpecialCharBranchFields {
    key401: string = '';
    level2: SpecialCharLeafFields = { key232: 1 };
}

class DeepSpecialCharObj {
    level1: SpecialCharBranchFields = { key401: '', level2: { key232: 1 } };
}

class EmptyArrFilterObj {
    valid: number[] = [];
    empty?: never[] = [];
}

class MultiPunctObj {
    en: string = '';
    de: string = '';
    fr: string = '';
}

class CubeRootCompare {
    numbers: number[] = [];
    roots: number[] = [];
}

function unicodeEmojiRoundtrip(): void {
    const originalStr: string = 'Hello 😊 World 🌟'
    const jsonStr: string = JSON.stringify(originalStr)
    let typ: Type = Type.of(new String())
    const parsedStr: string = JSON.parse<string>(jsonStr, typ) as string

    arktest.assertEQ(parsedStr, originalStr)
    arktest.assertEQ(parsedStr.includes('😊'), true)
    arktest.assertEQ(parsedStr.includes('🌟'), true)
}

function reviverReplaceContainer(): void {
    const originalObj: RootNestedReplaceObj = {
        container: { value: '123' }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new RootNestedReplaceObj())
    const parsedObj: RootNestedReplaceObj = JSON.parse<RootNestedReplaceObj>(jsonStr, (key: string, value: Any) => {
        if (key === 'container') {
            return { value: 'replaced' } as ReplacedContainerValue
        }
        return value
    }, typ) as RootNestedReplaceObj

    arktest.assertEQ(parsedObj.container.value, 'replaced')
    arktest.assertEQ(typeof parsedObj.container.value, 'string')
}

function unicodeAccentRoundtrip(): void {
    const originalStr: string = 'é (e\u0301)'
    const jsonStr: string = JSON.stringify(originalStr)
    let typ: Type = Type.of(new String())
    const parsedStr: string = JSON.parse<string>(jsonStr, typ) as string

    arktest.assertEQ(parsedStr, originalStr)
    arktest.assertEQ(parsedStr.includes('é'), true)
    arktest.assertEQ(parsedStr.includes('\u0301'), true)
}

function reviverComputeSum(): void {
    const originalObj: SumResultFields = { a: 5, b: 10, sum: 0 }
    const jsonStr: string = JSON.stringify(originalObj)
    let values: SumResultFields = {
        a: 0,
        b: 0,
        sum: 0
    }
    let typ: Type = Type.of(new SumResultFields())
    const parsedObj: SumResultFields = JSON.parse<SumResultFields>(jsonStr, (key: string, value: Any) => {
        if (key === 'a') {
            values.a = value as number
        }
        if (key === 'b') {
            values.b = value as number
        }
        if (key === '') {
            const obj = value as SumSourceFields
            return { a: obj.a, b: obj.b, sum: values.a + values.b } as SumResultFields
        }
        return value
    }, typ) as SumResultFields
    arktest.assertEQ(parsedObj.sum, 15)
    arktest.assertEQ(parsedObj.sum, parsedObj.a + parsedObj.b)
}

function reviverTruncateStr(): void {
    const originalObj: StrTruncateObj = {
        shortTest: 'hello',
        longTest: 'this is a very long string'
    }
    const jsonStr: string = JSON.stringify(originalObj,
        (key: string, value: Any) => typeof value === 'string' && (value as string).length > 10 ?
            (value as string).slice(0, 10) : value
    )
    let typ: Type = Type.of(new StrTruncateObj())
    const parsedObj: StrTruncateObj = JSON.parse<StrTruncateObj>(jsonStr, typ) as StrTruncateObj

    arktest.assertEQ(parsedObj.shortTest, 'hello')
    arktest.assertEQ(parsedObj.longTest, 'this is a ')
    arktest.assertEQ(parsedObj.longTest.length, 10)
}

function nestedNewlineRoundtrip(): void {
    const originalObj: NestedNewlineObj = {
        level1: { level2: 'Line1\nLine2\nLine3' }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new NestedNewlineObj())
    const parsedObj: NestedNewlineObj = JSON.parse<NestedNewlineObj>(jsonStr, typ) as NestedNewlineObj

    arktest.assertEQ(parsedObj.level1.level2, 'Line1\nLine2\nLine3')
    arktest.assertEQ(parsedObj.level1.level2.split('\n').length, 3)
}

function reviverRenameKey(): void {
    const originalObj: TempValueFields = { temp: 'value' }
    const jsonStr: string = JSON.stringify(originalObj, (key: string, value: Any) => {
        if (key === 'temp') {
            return { temp: (value as string) } as TempValueFields
        }
        if (key === '') {
            return { newKey: (value as TempValueFields).temp } as RenamePropObj
        }
        return value
    })
    let typ: Type = Type.of(new RenamePropObj())
    const parsedObj: RenamePropObj = JSON.parse<RenamePropObj>(jsonStr, typ) as RenamePropObj

    arktest.assertEQ(parsedObj.newKey, 'value')
}

function jsonElemStrToNum(): void {
    const originalArr: string[] = ['10', '20', '30']
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr,
        (key: string, value: jsonx.JsonElement) => key !== '' && typeof value === 'string' ?
            jsonx.JsonElement.createDouble(parseInt(value.asString(), 10)) : value)

    arktest.assertEQ(parsedArr[0].asString(), '10')
    arktest.assertEQ(parsedArr[1].asString(), '20')
    arktest.assertEQ(typeof parsedArr[0].asString(), 'string')
}

function specialCharNestedArr(): void {
    const originalArr: SpecialCharArr = [
        ['!@#', '$%^'],
        ['&*()', '[]{}']
    ]
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)

    arktest.assertEQ(parsedArr[0][0].asString(), '!@#')
    arktest.assertEQ(parsedArr[1][1].asString(), '[]{}')
}

function multiLangRoundtrip(): void {
    const originalObj: MultiLangObj = {
        en: 'Hello',
        zh: '你好',
        ja: 'こんにちは'
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new MultiLangObj())
    const parsedObj: MultiLangObj = JSON.parse<MultiLangObj>(jsonStr, typ) as MultiLangObj

    arktest.assertEQ(parsedObj.en, 'Hello')
    arktest.assertEQ(parsedObj.zh, '你好')
    arktest.assertEQ(parsedObj.ja, 'こんにちは')
}

function reviverTruncateArr(): void {
    const originalArr: number[] = [1, 2, 3, 4, 5]
    const jsonStr: string = JSON.stringify(originalArr, (key: string, value: Any) => {
        if (key === '' && Array.isArray(value) && (value as Array<number>).length > 3) {
            return (value as Array<number>).slice(0, 3)
        }
        return value
    })
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)

    arktest.assertEQ(parsedArr.asArray().length, 3)
    arktest.assertEQ(parsedArr[0].asDouble(), 1)
    arktest.assertEQ(parsedArr[2].asDouble(), 3)
}

function reviverComputeProduct(): void {
    const obj: ProductCompare = { numbers: [2, 3, 4], product: 0 }
    const json = JSON.stringify(obj)
    const parsed: jsonx.JsonElement = JSON.parseJsonElement(json, (k: string, v: jsonx.JsonElement) => {
        if (k === '') {
            let p = 1
            v['numbers'].asArray().forEach((n: jsonx.JsonElement): void => p *= n.asDouble())
            const original = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>)
            original.setElement('numbers', v['numbers'])
            original.setElement('product', jsonx.JsonElement.createDouble(p))
            return original
        }
        return v
    })
    arktest.assertEQ(parsed['product'].asDouble(), 24)
}

function zeroWidthJoinerRoundtrip(): void {
    const originalStr: string = 'a\u200Cb'
    const jsonStr: string = JSON.stringify(originalStr)
    let typ: Type = Type.of(new String())
    const parsedStr: string = JSON.parse<string>(jsonStr, typ) as string

    arktest.assertEQ(parsedStr, originalStr)
    arktest.assertEQ(parsedStr.includes('\u200C'), true)
    arktest.assertEQ(parsedStr.length, 3)
}

function controlCharRoundtrip(): void {
    const originalArr: string[] = ['\x01Start', 'End\x04']
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)

    arktest.assertEQ(parsedArr[0].asString(), '\x01Start')
    arktest.assertEQ(parsedArr[1].asString(), 'End\x04')
    arktest.assertEQ(parsedArr[0].asString().charCodeAt(0), 1)
}

function reviverDateConvert(): void {
    const originalObj: LogListObject = {
        logs: [
            { event: 'login', time: '2024-01-01T00:00:00Z' },
            { event: 'logout', time: '2024-01-01T01:00:00Z' }
        ]
    }
    const jsonStr: string = JSON.stringify(originalObj)
    const parsedObj: jsonx.JsonElement = JSON.parseJsonElement(jsonStr, (key: string, value: jsonx.JsonElement) => {
        if (key === 'time') {
            return jsonx.JsonElement.createDouble(new Date(value.asString()).getTime())
        }
        return value
    })

    arktest.assertEQ(parsedObj['logs'][0]['time'].asDouble(), 1704067200000)
    arktest.assertEQ(parsedObj['logs'][1]['time'].asDouble(), 1704070800000)
}

function familyEmojiRoundtrip(): void {
    const originalStr: string = '👨‍👩‍👧‍👦 Family: 👨‍💻 Developer'
    const jsonStr: string = JSON.stringify(originalStr)
    let typ: Type = Type.of(new String())
    const parsedStr: string = JSON.parse<string>(jsonStr, typ) as string
    arktest.assertEQ(parsedStr, originalStr)
    arktest.assertEQ(parsedStr.includes('👨‍👩‍👧‍👦'), true)
    arktest.assertEQ(parsedStr.includes('👨‍💻'), true)
}

function reviverCubeRoot(): void {
    const obj: CubeRootCompare = { numbers: [8, 27, 64], roots: [] }
    const json = JSON.stringify(obj)
    const parsed: jsonx.JsonElement = JSON.parseJsonElement(json, (k: string, v: jsonx.JsonElement) => {
        if (k === '') {
            const r: jsonx.JsonElement[] = []
            v['numbers'].asArray()
                .forEach((n: jsonx.JsonElement): void => r.push(
                    jsonx.JsonElement.createDouble(Math.cbrt(n.asDouble()))))
            const original = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>)
            original.setElement('numbers', v['numbers'])
            original.setElement('roots', jsonx.JsonElement.createArray(r))
            return original
        }
        return v
    })
    arktest.assertEQ(parsed['roots'][0].asDouble().toInt(), 2)
    arktest.assertEQ(parsed['roots'][1].asDouble().toInt(), 3)
}

function crlfNewlineRoundtrip(): void {
    const originalStr: string = 'Line1\r\nLine2\nLine3'
    const jsonStr: string = JSON.stringify(originalStr)
    let typ: Type = Type.of(new String())
    const parsedStr: string = JSON.parse<string>(jsonStr, typ) as string

    arktest.assertEQ(parsedStr, originalStr)
    arktest.assertEQ(parsedStr.split(new RegExp('\\r?\\n')).length, 3)
}

function reviverReverseEmail(): void {
    const originalObj: EncryptedObj = {
        username: 'user456',
        email: 'user@example.com'
    }
    const jsonStr: string = JSON.stringify(originalObj,
        (key: string, value: Any) => key === 'email' ? (value as string).split('').reverse().join('') : value
    )
    let typ: Type = Type.of(new EncryptedObj())
    const parsedObj: EncryptedObj = JSON.parse<EncryptedObj>(jsonStr,
        (key: string, value: Any) => key === 'email' ? (value as string).split('').reverse().join('') : value
        , typ) as EncryptedObj

    arktest.assertEQ(parsedObj.email, 'user@example.com')
    arktest.assertEQ((JSON.parse<EncryptedObj>(jsonStr, typ) as EncryptedObj).email, 'moc.elpmaxe@resu')
}

function deepSpecialKeyRoundtrip(): void {
    const originalObj: DeepSpecialCharObj = {
        level1: {
            key401: 'value$1',
            level2: { key232: 100 }
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new DeepSpecialCharObj())
    const parsedObj: DeepSpecialCharObj = JSON.parse<DeepSpecialCharObj>(jsonStr, typ) as DeepSpecialCharObj

    arktest.assertEQ(parsedObj.level1.key401, 'value$1')
    arktest.assertEQ(parsedObj.level1.level2.key232, 100)
}

function reviverRemoveEmptyArr(): void {
    const originalObj: EmptyArrFilterObj = {
        valid: [1, 2],
        empty: []
    }
    const jsonStr: string = JSON.stringify(originalObj)
    const parsedObj: jsonx.JsonElement = JSON.parseJsonElement(jsonStr,
        (key: string, value: jsonx.JsonElement) => value.jsonType === 2 && value.asArray().length === 0 ?
            jsonx.JsonElement.createUndefined() : value
    )

    arktest.assertEQ(parsedObj['valid'].asArray().length, 2)
    arktest.assertEQ(parsedObj['empty'].jsonType, 0)
}

function multiPunctRoundtrip(): void {
    const originalObj: MultiPunctObj = {
        en: 'Hello! How are you?',
        de: 'Hallo! Wie geht es dir?',
        fr: 'Bonjour! Comment allez-vous?'
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new MultiPunctObj())
    const parsedObj: MultiPunctObj = JSON.parse<MultiPunctObj>(jsonStr, typ) as MultiPunctObj

    arktest.assertEQ(parsedObj.en, 'Hello! How are you?')
    arktest.assertEQ(parsedObj.de, 'Hallo! Wie geht es dir?')
    arktest.assertEQ(parsedObj.fr, 'Bonjour! Comment allez-vous?')
}

function reviverMatrixMeta(): void {
    const originalArr: number[][] = [[1, 2], [3, 4], [5, 6]]
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedObj: jsonx.JsonElement = JSON.parseJsonElement(jsonStr, (key: string, value: jsonx.JsonElement) => {
        if (key === '') {
            const original = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>)
            original.setElement('rows', jsonx.JsonElement.createDouble(value.asArray().length))
            original.setElement('cols', jsonx.JsonElement.createDouble(value.asArray()[0].asArray().length || 0))
            original.setElement('data', value)
            return original
        }
        return value
    })
    arktest.assertEQ(parsedObj['rows'].asDouble(), 3)
    arktest.assertEQ(parsedObj['cols'].asDouble(), 2)
    arktest.assertEQ(parsedObj['data'][1][1].asDouble(), 4)
}
