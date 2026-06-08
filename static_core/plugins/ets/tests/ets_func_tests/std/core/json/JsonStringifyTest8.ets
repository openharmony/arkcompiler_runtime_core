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
    const suite = new arktest.ArkTestsuite('JSON stringify and parse text transform tests')
    suite.addTest('zero-width space round-trip', zeroWidthSpaceRoundtrip)
    suite.addTest('replacer masks passwords', replacerMaskPwdArr)
    suite.addTest('replacer num to string', numToStringReplacer)
    suite.addTest('flag emoji round-trip', flagEmojiRoundtrip)
    suite.addTest('replacer masks phone/email', replacerMaskPhoneEmail)
    suite.addTest('zero-width joiner round-trip', zwjRoundtrip)
    suite.addTest('NaN/Infinity serialize to null', nanInfToNull)
    suite.addTest('deep NaN/Infinity to null', deepNanInfToNull)
    suite.addTest('skin tone emoji round-trip', skinToneEmojiRoundtrip)
    suite.addTest('math symbols round-trip', mathSymbolRoundtrip)
    suite.addTest('multi-bracket text round-trip', multiBracketRoundtrip)
    suite.addTest('replacer clamps values', replacerClampValues)
    suite.addTest('sub-flag emoji round-trip', subflagEmojiRoundtrip)
    suite.addTest('replacer masks card number', replacerMaskCardNum)
    suite.addTest('multi-quote text round-trip', multiQuoteRoundtrip)
    suite.addTest('deep zero-width char round-trip', deepZeroWidthRoundtrip)
    suite.addTest('reviver validates api types', reviverValidateType)
    suite.addTest('food emoji round-trip', foodEmojiRoundtrip)
    suite.addTest('chemical symbols round-trip', chemicalSymbolRoundtrip)
    suite.addTest('music symbols round-trip', musicSymbolRoundtrip)
    suite.addTest('currency symbols round-trip', currencySymbolRoundtrip)
    suite.addTest('emoji nested object round-trip', emojiNestedObject)
    suite.addTest('reviver default values', reviverDefaultValues)
    suite.addTest('zero-width char obj round-trip', zeroWidthCharObjRoundtrip)
    suite.addTest('special char config round-trip', specialCharConfigRoundtrip)
    suite.addTest('Set to Array replacer', setToArrayReplacer)
    suite.addTest('file path round-trip', filePathRoundtrip)
    suite.addTest('query string round-trip', queryStringRoundtrip)
    suite.addTest('zero-width non-joiner round-trip', zeroWidthNonJoinerRoundtrip)
    suite.addTest('reviver clamps range', reviverClampRange)
    suite.addTest('CSS selector round-trip', cssSelectorRoundtrip)
    suite.addTest('CLI command round-trip', cliCommandRoundtrip)
    return suite.run()
}

class SensitiveUserRecord {
    id: number;
    name: string = '';
    password?: string = '';
}

class ArrSensitiveObj {
    users: SensitiveUserRecord[] = [];
}

class NumToStringObj {
    a: string = '';
    b: string = '';
    c: string = '';
}

class MaskedObj {
    phone: string = '';
    email: string = '';
}

class SpecialNumFilterObj {
    nan: null = null;
    inf: null = null;
    normal: number;
}

class ParsedSpecialNumberLeaf {
    inf: null = null;
    num: number;
}

class ParsedSpecialNumberBranch {
    nan: null = null;
    level2: ParsedSpecialNumberLeaf = { inf: null, num: 1 };
}

class DeepSpecialNumObj {
    level1: ParsedSpecialNumberBranch = { nan: null, level2: { inf: null, num: 1 } };
}

class MathSymbolNestedFields {
    cxToAd: number;
}

class MathSymbolFormulaFields {
    axToBb: string = '';
    nested: MathSymbolNestedFields = { cxToAd: 1 };
}

class MathSymbolObj {
    formulas: MathSymbolFormulaFields = { axToBb: '', nested: { cxToAd: 1 } };
}

class MultiBracketObj {
    text: string = '';
}

class ValueClampObj {
    score: number;
    age: number;
}

class PartialMaskObj {
    cardNumber: string = '';
    phone: string = '';
}

class MultiQuoteObj {
    text: string = '';
}

class ZeroWidthLeafFields {
    keyx: number;
}

class ZeroWidthBranchFields {
    keyy: string = '';
    level2: ZeroWidthLeafFields = { keyx: 1 };
}

class DeepZeroWidthObj {
    level1: ZeroWidthBranchFields = { keyy: '', level2: { keyx: 1 } };
}

class ResponseDataFields {
    message: string = '';
    timestamp: number;
}

class ApiResponse {
    code: number;
    data: ResponseDataFields = { message: '', timestamp: 1 };
}

class ChemicalNestedFields {
    CO2: number;
}

class ChemicalElementFields {
    H2O: string = '';
    nested: ChemicalNestedFields = { CO2: 0 };
}

class ChemicalSymbolObj {
    elements: ChemicalElementFields = { H2O: '', nested: { CO2: 0 } };
}

class MusicNestedFields {
    Dx23: number;
}

class MusicNoteFields {
    Cx23: string = '';
    nested: MusicNestedFields = { Dx23: 0 };
}

class MusicSymbolObj {
    notes: MusicNoteFields = { Cx23: '', nested: { Dx23: 0 } };
}

class OriginalSpecialNumberFields {
    nan: number;
    inf: number;
    normal: number;
}

class OriginalSpecialNumberLeaf {
    inf: number;
    num: number;
}

class OriginalSpecialNumberBranch {
    nan: number;
    level2: OriginalSpecialNumberLeaf = { inf: 1, num: 1 };
}

class OriginalDeepSpecialNumbers {
    level1: OriginalSpecialNumberBranch = { nan: 1, level2: { inf: 1, num: 1 } };
}

class EmojiUserStatus {
    name: string = '';
    status: string = '';
}

class EmojiNestedObj {
    user: EmojiUserStatus = new EmojiUserStatus();
}

class ConfigRuntimeFields {
    enabled: boolean;
    timeout: number;
}

class DefaultConfigFields {
    theme: string = '';
    layout: string = '';
    nested: ConfigRuntimeFields = new ConfigRuntimeFields();
}

class DefaultValueDeepObj {
    config: DefaultConfigFields = new DefaultConfigFields();
}

class ZeroWidthTextFields {
    withJoiner: string = '';
    withSpace: string = '';
}

class ZeroWidthCharObj {
    text: ZeroWidthTextFields = new ZeroWidthTextFields();
}

class SpecialCharNestedSettings {
    enable_ssl: boolean;
    timeout_ms: number;
}

class SpecialCharSettings {
    api_endpoint: string = '';
    max_size: string = '';
    nested: SpecialCharNestedSettings = new SpecialCharNestedSettings();
}

class SpecialCharConfigObj {
    settings: SpecialCharSettings = new SpecialCharSettings();
}

class ZeroWidthNonJoinerNestedText {
    combined: string = '';
}

class ZeroWidthNonJoinerTexts {
    original: string = '';
    nested: ZeroWidthNonJoinerNestedText = new ZeroWidthNonJoinerNestedText();
}

class ZeroWidthNonJoinerObj {
    texts: ZeroWidthNonJoinerTexts = new ZeroWidthNonJoinerTexts();
}

class RangeLimitedSettings {
    volume: number;
    brightness: number;
}

class ValueRangeCheckObj {
    settings: RangeLimitedSettings = new RangeLimitedSettings();
}

function zeroWidthSpaceRoundtrip(): void {
    const originalArr: string[][] = [
        ['a\u200B', 'b'],
        ['c', 'd\u200B']
    ]
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)

    arktest.assertEQ(parsedArr[0][0].asString(), 'a\u200B')
    arktest.assertEQ(parsedArr[1][1].asString(), 'd\u200B')
    arktest.assertEQ(parsedArr[0][0].asString().includes('\u200B'), true)
}

function replacerMaskPwdArr(): void {
    const originalObj: ArrSensitiveObj = {
        users: [
            { id: 1, name: 'Dave', password: 'pass1' },
            { id: 2, name: 'Eve', password: 'pass2' }
        ]
    }
    const jsonStr: string =
        JSON.stringify(originalObj, (key: string, value: Any) => key === 'password' ? undefined : value)
    const parsedObj: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)

    arktest.assertEQ(parsedObj['users'][0].tryGetElement('password'), undefined)
    arktest.assertEQ(parsedObj['users'][1]['name'].asString(), 'Eve')
}

function numToStringReplacer(): void {
    const originalObj: NumToStringObj = { a: '10', b: '20', c: '30' }
    const jsonStr: string = JSON.stringify(originalObj,
        (key: string, value: Any) => typeof value === 'number' ? (value as NumToStringObj).toString() : value
    )
    let typ: Type = Type.of(new NumToStringObj())
    const parsedObj: NumToStringObj = JSON.parse<NumToStringObj>(jsonStr, typ) as NumToStringObj

    arktest.assertEQ(parsedObj.a, '10')
    arktest.assertEQ(parsedObj.b, '20')
    arktest.assertEQ(typeof parsedObj.c, 'string')
}

function flagEmojiRoundtrip(): void {
    const originalStr: string = '🇨🇳 China 🇺🇸 USA'
    const jsonStr: string = JSON.stringify(originalStr)
    let typ: Type = Type.of(new String())
    const parsedStr: string = JSON.parse<string>(jsonStr, typ) as string

    arktest.assertEQ(parsedStr, originalStr)
    arktest.assertEQ(parsedStr.includes('🇨🇳'), true)
    arktest.assertEQ(parsedStr.includes('🇺🇸'), true)
}

function replacerMaskPhoneEmail(): void {
    const originalObj: MaskedObj = {
        phone: '13800138000',
        email: 'user@example.com'
    }
    const jsonStr: string = JSON.stringify(originalObj, (key: string, value: Any) => {
        if (key === 'phone' && typeof value === 'string') {
            return (value as string).replace(new RegExp('(\\d{3})\\d*(\\d{4})'), '$1****$2')
        }
        if (key === 'email' && typeof value === 'string') {
            return (value as string).replace(new RegExp('(.{2}).*@'), '$1***@')
        }
        return value
    })
    let typ: Type = Type.of(new MaskedObj())
    const parsedObj: MaskedObj = JSON.parse<MaskedObj>(jsonStr, typ) as MaskedObj

    arktest.assertEQ(parsedObj.phone, '138****8000')
    arktest.assertEQ(parsedObj.email, 'us***@example.com')
}

function zwjRoundtrip(): void {
    const originalStr: string = 'a\u200Db'
    const jsonStr: string = JSON.stringify(originalStr)
    let typ: Type = Type.of(new String())
    const parsedStr: string = JSON.parse<string>(jsonStr, typ) as string
    arktest.assertEQ(parsedStr, originalStr)
    arktest.assertEQ(parsedStr.includes('\u200D'), true)
    arktest.assertEQ(parsedStr.length, 3)
}

function nanInfToNull(): void {
    const originalObj: OriginalSpecialNumberFields = {
        nan: NaN,
        inf: Infinity,
        normal: 100
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new SpecialNumFilterObj())
    const parsedObj: SpecialNumFilterObj = JSON.parse<SpecialNumFilterObj>(jsonStr, typ) as SpecialNumFilterObj

    arktest.assertEQ(parsedObj.nan, null)
    arktest.assertEQ(parsedObj.inf, null)
    arktest.assertEQ(parsedObj.normal, 100)
}

function deepNanInfToNull(): void {
    const originalObj: OriginalDeepSpecialNumbers = {
        level1: {
            nan: NaN,
            level2: { inf: Infinity, num: 200 }
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new DeepSpecialNumObj())
    const parsedObj: DeepSpecialNumObj = JSON.parse<DeepSpecialNumObj>(jsonStr, typ) as DeepSpecialNumObj

    arktest.assertEQ(parsedObj.level1.nan, null)
    arktest.assertEQ(parsedObj.level1.level2.inf, null)
    arktest.assertEQ(parsedObj.level1.level2.num, 200)
}

function skinToneEmojiRoundtrip(): void {
    const originalStr: string = '👨🏿‍💻 Dark skin developer'
    const jsonStr: string = JSON.stringify(originalStr)
    let typ: Type = Type.of(new String())
    const parsedStr: string = JSON.parse<string>(jsonStr, typ) as string

    arktest.assertEQ(parsedStr, originalStr)
    arktest.assertEQ(parsedStr.includes('👨🏿‍💻'), true)
}

function mathSymbolRoundtrip(): void {
    const originalObj: MathSymbolObj = {
        formulas: {
            axToBb: 'sum',
            nested: { cxToAd: 100 }
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new MathSymbolObj())
    const parsedObj: MathSymbolObj = JSON.parse<MathSymbolObj>(jsonStr, typ) as MathSymbolObj

    arktest.assertEQ(parsedObj.formulas.axToBb, 'sum')
    arktest.assertEQ(parsedObj.formulas.nested.cxToAd, 100)
}

function multiBracketRoundtrip(): void {
    const originalObj: MultiBracketObj = {
        text: '中文（括号）、英文()、日文（かっこ）'
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new MultiBracketObj())
    const parsedObj: MultiBracketObj = JSON.parse<MultiBracketObj>(jsonStr, typ) as MultiBracketObj
    arktest.assertEQ(parsedObj.text, '中文（括号）、英文()、日文（かっこ）')
    arktest.assertEQ(parsedObj.text.includes('（'), true)
    arktest.assertEQ(parsedObj.text.includes('()'), true)
}

function replacerClampValues(): void {
    const originalObj: ValueClampObj = { score: 150, age: -5 }
    const jsonStr: string = JSON.stringify(originalObj, (key: string, value: Any) => {
        if (key === 'score') {
            return Math.min(100, value as number)
        }
        if (key === 'age') {
            return Math.max(0, value as number)
        }
        return value
    })
    let typ: Type = Type.of(new ValueClampObj())
    const parsedObj: ValueClampObj = JSON.parse<ValueClampObj>(jsonStr, typ) as ValueClampObj

    arktest.assertEQ(parsedObj.score, 100)
    arktest.assertEQ(parsedObj.age, 0)
}

function subflagEmojiRoundtrip(): void {
    const originalStr: string = '🏴󠁧󠁢󠁥󠁮󠁧󠁿 UK 🇪🇸 Spain'
    const jsonStr: string = JSON.stringify(originalStr)
    let typ: Type = Type.of(new String())
    const parsedStr: string = JSON.parse<string>(jsonStr, typ) as string

    arktest.assertEQ(parsedStr, originalStr)
    arktest.assertEQ(parsedStr.includes('🏴󠁧󠁢󠁥󠁮󠁧󠁿'), true)
    arktest.assertEQ(parsedStr.includes('🇪🇸'), true)
}

function replacerMaskCardNum(): void {
    const originalObj: PartialMaskObj = {
        cardNumber: '6222021234567890123',
        phone: '13800138000'
    }
    const jsonStr: string = JSON.stringify(originalObj, (key: string, value: Any) => {
        if (key === 'cardNumber' && typeof value === 'string') {
            return (value as string).replace(new RegExp('^(\\d{4})\\d*(\\d{4})$'), '$1********$2')
        }
        return value
    })
    let typ: Type = Type.of(new PartialMaskObj())
    const parsedObj: PartialMaskObj = JSON.parse<PartialMaskObj>(jsonStr, typ) as PartialMaskObj
    arktest.assertEQ(parsedObj.cardNumber, '6222********0123')
    arktest.assertEQ(parsedObj.phone, '13800138000')
}

function multiQuoteRoundtrip(): void {
    const originalObj: MultiQuoteObj = {
        text: '中文“引号”、英文"引号"、日文「引用」'
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new MultiQuoteObj())
    const parsedObj: MultiQuoteObj = JSON.parse<MultiQuoteObj>(jsonStr, typ) as MultiQuoteObj
    arktest.assertEQ(parsedObj.text, '中文“引号”、英文"引号"、日文「引用」')
    arktest.assertEQ(parsedObj.text.includes('“'), true)
    arktest.assertEQ(parsedObj.text.includes('「'), true)
}

function deepZeroWidthRoundtrip(): void {
    const originalObj: DeepZeroWidthObj = {
        level1: {
            keyy: 'value\u200C1',
            level2: { keyx: 100 }
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new DeepZeroWidthObj())
    const parsedObj: DeepZeroWidthObj = JSON.parse<DeepZeroWidthObj>(jsonStr, typ) as DeepZeroWidthObj

    arktest.assertEQ(parsedObj.level1.keyy, 'value\u200C1')
    arktest.assertEQ(parsedObj.level1.level2.keyx, 100)
}

function reviverValidateType(): void {
    const originalObj: ApiResponse = {
        code: 200,
        data: { message: 'success', timestamp: Date.now() }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new ApiResponse())
    const parsedObj: ApiResponse = JSON.parse<ApiResponse>(jsonStr, (key: string, value: Any) => {
        if (key === 'code' && typeof value !== 'number') {
            throw new Error('code must be number')
        }
        if (key === 'message' && typeof value !== 'string') {
            throw new Error('message must be string')
        }
        return value
    }, typ) as ApiResponse
    arktest.assertEQ(parsedObj.code, 200)
    arktest.assertEQ(typeof parsedObj.data.timestamp, 'number')
}

function foodEmojiRoundtrip(): void {
    const originalStr: string = '🍕 Pizza 🍟 Fries 🍔 Burger'
    const jsonStr: string = JSON.stringify(originalStr)
    let typ: Type = Type.of(new String())
    const parsedStr: string = JSON.parse<string>(jsonStr, typ) as string

    arktest.assertEQ(parsedStr, originalStr)
    arktest.assertEQ(parsedStr.includes('🍕'), true)
    arktest.assertEQ(parsedStr.includes('🍔'), true)
}

function chemicalSymbolRoundtrip(): void {
    const originalObj: ChemicalSymbolObj = {
        elements: {
            H2O: 'Water',
            nested: { CO2: 100 }
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new ChemicalSymbolObj())
    const parsedObj: ChemicalSymbolObj = JSON.parse<ChemicalSymbolObj>(jsonStr, typ) as ChemicalSymbolObj

    arktest.assertEQ(parsedObj.elements.H2O, 'Water')
    arktest.assertEQ(parsedObj.elements.nested.CO2, 100)
}

function musicSymbolRoundtrip(): void {
    const originalObj: MusicSymbolObj = {
        notes: {
            Cx23: 'Middle C Sharp',
            nested: { Dx23: 200 }
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    let typ: Type = Type.of(new MusicSymbolObj())
    const parsedObj: MusicSymbolObj = JSON.parse<MusicSymbolObj>(jsonStr, typ) as MusicSymbolObj

    arktest.assertEQ(parsedObj.notes.Cx23, 'Middle C Sharp')
    arktest.assertEQ(parsedObj.notes.nested.Dx23, 200)
}

function currencySymbolRoundtrip(): void {
    const originalArr: string[] = ['$100', '€50', '¥1000', '£20']
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(parsedArr[0].asString(), '$100')
    arktest.assertEQ(parsedArr[1].asString(), '€50')
    arktest.assertEQ(parsedArr[2].asString(), '¥1000')
}

function emojiNestedObject(): void {
    const originalObj: EmojiNestedObj = {
        user: {
            name: '😊 Alice',
            status: '🎉 Online'
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    const parsedObj: EmojiNestedObj = JSON.parse<EmojiNestedObj>(
        jsonStr, Class.from<EmojiNestedObj>()) as EmojiNestedObj

    arktest.assertEQ(parsedObj.user.name, '😊 Alice')
    arktest.assertEQ(parsedObj.user.status, '🎉 Online')
}

function reviverDefaultValues(): void {
    const originalObj: DefaultValueDeepObj = {
        config: {
            theme: 'light',
            nested: { enabled: true }
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    const parsedObj: DefaultValueDeepObj = JSON.parse<DefaultValueDeepObj>(jsonStr, (key: string, value: Any) => {
        if (key === 'layout') {
            return value ?? 'grid'
        }
        if (key === 'timeout') {
            return value ?? 3000
        }
        return value
    }, Class.from<DefaultValueDeepObj>()) as DefaultValueDeepObj
    arktest.assertEQ(parsedObj.config.layout, '')
    arktest.assertEQ(parsedObj.config.nested.timeout, 0)
}

function zeroWidthCharObjRoundtrip(): void {
    const originalObj: ZeroWidthCharObj = {
        text: {
            withJoiner: 'a\u200Db',
            withSpace: 'x\u200By'
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    const parsedObj: ZeroWidthCharObj = JSON.parse<ZeroWidthCharObj>(jsonStr,
        Type.from<ZeroWidthCharObj>()) as ZeroWidthCharObj

    arktest.assertEQ(parsedObj.text.withJoiner, 'a\u200Db')
    arktest.assertEQ(parsedObj.text.withSpace, 'x\u200By')
}

function specialCharConfigRoundtrip(): void {
    const originalObj: SpecialCharConfigObj = {
        settings: {
            api_endpoint: 'https://api.example.com/v1',
            max_size: '10MB',
            nested: { 'enable_ssl': true, timeout_ms: 5000 }
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    const parsedObj: SpecialCharConfigObj = JSON.parse<SpecialCharConfigObj>(jsonStr,
        Type.from<SpecialCharConfigObj>()) as SpecialCharConfigObj

    arktest.assertEQ(parsedObj.settings.api_endpoint, 'https://api.example.com/v1')
    arktest.assertEQ(parsedObj.settings.nested.timeout_ms, 5000)
}

function setToArrayReplacer(): void {
    const originalSet: Set<string> = new Set<string>(['b', 'a', 'c'])
    const jsonStr: string = JSON.stringify(originalSet, (key: string, value: Any) =>
        value instanceof Set ? Array.from(value) : value
    )
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)

    arktest.assertEQ(JSON.stringify(parsedArr[1].asString()), JSON.stringify('a'))
}

function filePathRoundtrip(): void {
    const originalArr: string[] = [
        '/home/user/docs/file name.txt',
        '/var/log/app.log',
        'C:\\Program Files\\app.exe'
    ]
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)

    arktest.assertEQ(parsedArr[0].asString(), '/home/user/docs/file name.txt')
    arktest.assertEQ(parsedArr[2].asString(), 'C:\\Program Files\\app.exe')
}

function queryStringRoundtrip(): void {
    const originalArr: string[] = [
        '?q=json&page=1&limit=10',
        '?category=books&sort=price&order=asc'
    ]
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)

    arktest.assertEQ(parsedArr[0].asString(), '?q=json&page=1&limit=10')
    arktest.assertEQ(parsedArr[1].asString(), '?category=books&sort=price&order=asc')
}

function zeroWidthNonJoinerRoundtrip(): void {
    const originalObj: ZeroWidthNonJoinerObj = {
        texts: {
            original: 'a\u200Cb',
            nested: { combined: 'x\u200Cy' }
        }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    const parsedObj: ZeroWidthNonJoinerObj = JSON.parse<ZeroWidthNonJoinerObj>(jsonStr,
        Class.from<ZeroWidthNonJoinerObj>()) as ZeroWidthNonJoinerObj
    arktest.assertEQ(parsedObj.texts.original, 'a\u200Cb')
    arktest.assertEQ(parsedObj.texts.nested.combined, 'x\u200Cy')
}

function reviverClampRange(): void {
    const originalObj: ValueRangeCheckObj = {
        settings: { volume: 120, brightness: -10 }
    }
    const jsonStr: string = JSON.stringify(originalObj)
    const parsedObj: ValueRangeCheckObj = JSON.parse<ValueRangeCheckObj>(jsonStr, (key: string, value: Any) => {
        if (key === 'volume' || key === 'brightness') {
            return Math.max(0, Math.min(100, value as number))
        }
        return value
    }, Type.from<ValueRangeCheckObj>()) as ValueRangeCheckObj
    arktest.assertEQ(parsedObj.settings.volume, 100)
    arktest.assertEQ(parsedObj.settings.brightness, 0)
}

function cssSelectorRoundtrip(): void {
    const originalArr: string[] = [
        '.class-name > .child:nth-child(2)',
        '#id-name[data-attr=\'value\']',
        'div:not(.exclude)'
    ]
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)

    arktest.assertEQ(parsedArr[0].asString(), '.class-name > .child:nth-child(2)')
    arktest.assertEQ(parsedArr[1].asString(), '#id-name[data-attr=\'value\']')
}

function cliCommandRoundtrip(): void {
    const originalArr: string[] = [
        'npm install --save-dev typescript',
        'git commit -m \'fix: resolve issue\'',
        'docker run -p 8080:80 --name app'
    ]
    const jsonStr: string = JSON.stringify(originalArr)
    const parsedArr: jsonx.JsonElement = JSON.parseJsonElement(jsonStr)

    arktest.assertEQ(parsedArr[0].asString(), 'npm install --save-dev typescript')
    arktest.assertEQ(parsedArr[1].asString(), 'git commit -m \'fix: resolve issue\'')
}
