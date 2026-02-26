/**
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

function main(): int {
    const suite = new arktest.ArkTestsuite("json.JSON.parseJsonRecord")

    suite.addTest("Parse empty object", testParseEmptyObject)
    suite.addTest("Parse simple object", testParseSimpleObject)
    suite.addTest("Parse nested object", testParseNestedObject)
    suite.addTest("Parse object with multiple types", testParseMixedObject)

    suite.addTest("Parse object with empty array", testParseEmptyArray)
    suite.addTest("Parse object with simple array", testParseSimpleArray)
    suite.addTest("Parse object with mixed array", testParseMixedArray)
    suite.addTest("Parse object with nested array", testParseNestedArray)
    suite.addTest("Parse object with array of objects", testParseArrayWithObjects)

    suite.addTest("Parse with spaces", testParseWithSpaces)
    suite.addTest("Parse with newlines", testParseWithNewlines)
    suite.addTest("Parse with tabs", testParseWithTabs)
    suite.addTest("Parse with mixed whitespace", testParseWithMixedWhitespace)

    suite.addTest("Parse escaped quotes", testParseEscapedQuotes)
    suite.addTest("Parse escaped backslash", testParseEscapedBackslash)
    suite.addTest("Parse escaped unicode", testParseEscapedUnicode)

    suite.addTest("Parse chinese characters", testParseChinese)
    suite.addTest("Parse emoji", testParseEmoji)

    suite.addTest("Error: unexpected end", testErrorUnexpectedEnd)
    suite.addTest("Error: expected colon", testErrorExpectedColon)
    suite.addTest("Error: expected comma", testErrorExpectedComma)
    suite.addTest("Error: unterminated string", testErrorUnterminatedString)
    suite.addTest("Error: invalid escape", testErrorInvalidEscape)
    suite.addTest("Error: invalid unicode", testErrorInvalidUnicode)

    suite.addTest("Reject: array root", testRejectArrayRoot)
    suite.addTest("Reject: string root", testRejectStringRoot)
    suite.addTest("Reject: number root", testRejectNumberRoot)
    suite.addTest("Reject: boolean root", testRejectBooleanRoot)
    suite.addTest("Reject: null root", testRejectNullRoot)
    suite.addTest("Reject: empty input", testRejectEmptyInput)
    suite.addTest("Reject: whitespace only", testRejectWhitespaceOnly)

    suite.addTest("Parse deeply nested object", testParseDeeplyNested)
    suite.addTest("Parse large object", testParseLargeObject)

    suite.addTest("Parse bigint with ALWAYS_PARSE_AS_BIGINT", testParseBigIntAlwaysMode)
    suite.addTest("Parse bigint with PARSE_AS_BIGINT", testParseBigIntMode)
    suite.addTest("Parse bigint with DEFAULT", testParseBigIntDefaultMode)
    suite.addTest("Parse nested object with bigint", testParseNestedObjectWithBigInt)
    suite.addTest("Parse array with bigint", testParseArrayWithBigInt)
    return suite.run()
}

function testParseEmptyObject(): void {
    const jsonStr = '{}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 0, "Empty object should have size 0")
}

function testParseSimpleObject(): void {
    const jsonStr = '{"name": "Alice", "age": 30}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 2, "Record should have 2 entries")
    arktest.assertEQ(record.get("name"), "Alice", "String value should be correct")
    arktest.assertEQ(record.get("age"), 30, "Integer value should be correct")
}

function testParseNestedObject(): void {
    const jsonStr = '{"user": {"name": "Bob", "age": 25}}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    let user = record.get("user") as Record<string, JsonRecordType>
    arktest.assertNE(user, undefined, "Nested object should exist")
    arktest.assertEQ(user.size, 2, "Nested object should have 2 entries")
    arktest.assertEQ(user.get("name"), "Bob", "Nested string should be correct")
    arktest.assertEQ(user.get("age"), 25, "Nested integer should be correct")
}

function testParseMixedObject(): void {
    const jsonStr = '{"str": "text", "num": 123, "bool": true, "nullVal": null}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 4, "Record should have 4 entries")
    arktest.assertEQ(record.get("str"), "text", "String value correct")
    arktest.assertEQ(record.get("num"), 123, "Number value correct")
    arktest.assertEQ(record.get("bool"), true, "Boolean value correct")
    arktest.assertEQ(record.get("nullVal"), null, "Null value correct")
}

function testParseEmptyArray(): void {
    const jsonStr = '{"arr": []}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    let arr = record.get("arr") as Array<JsonRecordType>
    arktest.assertNE(arr, undefined, "Array should exist")
    arktest.assertEQ(arr.length, 0, "Array should be empty")
}

function testParseSimpleArray(): void {
    const jsonStr = '{"arr": [1, 2, 3]}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    let arr = record.get("arr") as Array<JsonRecordType>
    arktest.assertNE(arr, undefined, "Array should exist")
    arktest.assertEQ(arr.length, 3, "Array should have 3 elements")
    arktest.assertEQ(arr[0], 1, "First element correct")
    arktest.assertEQ(arr[1], 2, "Second element correct")
    arktest.assertEQ(arr[2], 3, "Third element correct")
}

function testParseMixedArray(): void {
    const jsonStr = '{"arr": [1, "two", true, null]}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    let arr = record.get("arr") as Array<JsonRecordType>
    arktest.assertNE(arr, undefined, "Array should exist")
    arktest.assertEQ(arr.length, 4, "Array should have 4 elements")
    arktest.assertEQ(arr[0], 1, "Integer element correct")
    arktest.assertEQ(arr[1], "two", "String element correct")
    arktest.assertEQ(arr[2], true, "Boolean element correct")
    arktest.assertEQ(arr[3], null, "Null element correct")
}

function testParseNestedArray(): void {
    const jsonStr = '{"arr": [[1, 2], [3, 4]]}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    let arr = record.get("arr") as Array<JsonRecordType>
    arktest.assertNE(arr, undefined, "Array should exist")
    arktest.assertEQ(arr.length, 2, "Nested array should have 2 elements")
    let inner1 = arr[0] as Array<JsonRecordType>
    let inner2 = arr[1] as Array<JsonRecordType>
    arktest.assertEQ(inner1.length, 2, "First inner array length correct")
    arktest.assertEQ(inner1[0], 1, "First element correct")
    arktest.assertEQ(inner1[1], 2, "Second element correct")
    arktest.assertEQ(inner2.length, 2, "Second inner array length correct")
    arktest.assertEQ(inner2[0], 3, "Third element correct")
    arktest.assertEQ(inner2[1], 4, "Fourth element correct")
}

function testParseArrayWithObjects(): void {
    const jsonStr = '{"arr": [{"id": 1}, {"id": 2}]}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    let arr = record.get("arr") as Array<JsonRecordType>
    arktest.assertNE(arr, undefined, "Array should exist")
    arktest.assertEQ(arr.length, 2, "Array should have 2 elements")

    let obj1 = arr[0] as Record<string, JsonRecordType>
    arktest.assertNE(obj1, undefined, "First object should exist")
    arktest.assertEQ(obj1.get("id"), 1, "First object id correct")

    let obj2 = arr[1] as Record<string, JsonRecordType>
    arktest.assertNE(obj2, undefined, "Second object should exist")
    arktest.assertEQ(obj2.get("id"), 2, "Second object id correct")
}

function testParseWithSpaces(): void {
    const jsonStr = '{ "name" : "Alice" , "age" : 30 }'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 2, "Record should have 2 entries")
    arktest.assertEQ(record.get("name"), "Alice", "Value with spaces correct")
    arktest.assertEQ(record.get("age"), 30, "Value with spaces correct")
}

function testParseWithNewlines(): void {
    const jsonStr = '{\n"name"\n:\n"Alice"\n,\n"age"\n:\n30\n}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 2, "Record should have 2 entries")
    arktest.assertEQ(record.get("name"), "Alice", "Value with newlines correct")
    arktest.assertEQ(record.get("age"), 30, "Value with newlines correct")
}

function testParseWithTabs(): void {
    const jsonStr = '{"name"\t:\t"Alice"\t,\t"age"\t:\t30}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 2, "Record should have 2 entries")
    arktest.assertEQ(record.get("name"), "Alice", "Value with tabs correct")
    arktest.assertEQ(record.get("age"), 30, "Value with tabs correct")
}

function testParseWithMixedWhitespace(): void {
    const jsonStr = '{ \n \t "name" \r\n : \t \n "Alice" \r\n , \t \n "age" \r\n : \t \n 30 \n }'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 2, "Record should have 2 entries")
    arktest.assertEQ(record.get("name"), "Alice", "Value with mixed whitespace correct")
    arktest.assertEQ(record.get("age"), 30, "Value with mixed whitespace correct")
}

function testParseEscapedQuotes(): void {
    const jsonStr = '{"text": "He said \\"hello\\""}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    arktest.assertEQ(record.get("text"), "He said \"hello\"", "Escaped quotes parsed correctly")
}

function testParseEscapedBackslash(): void {
    const jsonStr = '{"path": "C:\\\\Users\\\\Name"}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    arktest.assertEQ(record.get("path"), "C:\\Users\\Name", "Escaped backslash parsed correctly")
}

function testParseEscapedUnicode(): void {
    const jsonStr = '{"unicode": "\\u4F60\\u597D"}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    arktest.assertEQ(record.get("unicode"), "你好", "Escaped unicode parsed correctly")
}

function testParseChinese(): void {
    const jsonStr = '{"name": "张三", "city": "北京"}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 2, "Record should have 2 entries")
    arktest.assertEQ(record.get("name"), "张三", "Chinese characters parsed correctly")
    arktest.assertEQ(record.get("city"), "北京", "Chinese characters parsed correctly")
}

function testParseEmoji(): void {
    const jsonStr = '{"emoji": "😀🎉"}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    arktest.assertEQ(record.get("emoji"), "😀🎉", "Emoji parsed correctly")
}

function testErrorUnexpectedEnd(): void {
    const jsonStr = '{"name"'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testErrorExpectedColon(): void {
    const jsonStr = '{"name" "Alice"}'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testErrorExpectedComma(): void {
    const jsonStr = '{"name": "Alice", "age" 30}'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testErrorUnterminatedString(): void {
    const jsonStr = '{"name": "Alice"'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testErrorInvalidEscape(): void {
    const jsonStr = '{"text": "Hello\\x"}'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testErrorInvalidUnicode(): void {
    const jsonStr = '{"text": "\\u123G"}'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testRejectArrayRoot(): void {
    const jsonStr = '[1, 2, 3]'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testRejectStringRoot(): void {
    const jsonStr = '"not an object"'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testRejectNumberRoot(): void {
    const jsonStr = '12345'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testRejectBooleanRoot(): void {
    const jsonStr = 'true'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testRejectNullRoot(): void {
    const jsonStr = 'null'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testRejectEmptyInput(): void {
    const jsonStr = ''
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testRejectWhitespaceOnly(): void {
    const jsonStr = '   \n\t  '
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr)
    })
}

function testParseDeeplyNested(): void {
    const jsonStr = '{"a": {"b": {"c": {"d": {"value": 42}}}}}'
    const record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    let a = record.get("a") as Record<string, JsonRecordType>
    let b = a.get("b") as Record<string, JsonRecordType>
    let c = b.get("c") as Record<string, JsonRecordType>
    let d = c.get("d") as Record<string, JsonRecordType>
    arktest.assertEQ(d.get("value"), 42, "Deeply nested value correct")
}

function testParseLargeObject(): void {
    let jsonStr = '{'
    for (let i = 0; i < 100; i++) {
        if (i > 0) {
            jsonStr += ','
        }
        jsonStr += `"key${i}": "value${i}"`
    }
    jsonStr += '}'

    let record = JSON.parseJsonRecord(jsonStr)

    arktest.assertEQ(record.size, 100, "Large object should have 100 entries")
    arktest.assertEQ(record.get("key0"), "value0", "First value correct")
    arktest.assertEQ(record.get("key99"), "value99", "Last value correct")
}

function testParseBigIntAlwaysMode(): void {
    const opt: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT} as jsonx.ParseOptions

    const jsonStr = '{"value": 9007199254740992}'
    const record = JSON.parseJsonRecord(jsonStr, opt)
    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    arktest.assertEQ(record.get("value"), 9007199254740992n, "BigInt value correct")

    const jsonStr2 = '{"big": 9223372036854775808}'
    const record2 = JSON.parseJsonRecord(jsonStr2, opt)
    arktest.assertEQ(record2.get("big"), 9223372036854775808n, "Large BigInt value correct")

    const jsonStr3 = '{"negative": -9007199254740992}'
    const record3 = JSON.parseJsonRecord(jsonStr3, opt)
    arktest.assertEQ(record3.get("negative"), -9007199254740992n, "Negative BigInt value correct")

    const jsonStr4 = '{"zero": 0}'
    const record4 = JSON.parseJsonRecord(jsonStr4, opt)
    arktest.assertEQ(record4.get("zero"), 0n, "Zero BigInt value correct")

    const jsonStr5 = '{"float": 123.456}'
    const record5 = JSON.parseJsonRecord(jsonStr5, opt)
    arktest.assertEQ(record5.get("float"), 123.456, "Float value should be double, not affected by option")
}

function testParseBigIntMode(): void {
    const opt: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.PARSE_AS_BIGINT} as jsonx.ParseOptions

    const jsonStr = '{"safe": 9007199254740991}'
    const record = JSON.parseJsonRecord(jsonStr, opt)
    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    arktest.assertEQ(record.get("safe"), 9007199254740991, "Safe integer parsed as long")

    const jsonStr2 = '{"unsafe": 9007199254740992}'
    const record2 = JSON.parseJsonRecord(jsonStr2, opt)
    arktest.assertEQ(record2.get("unsafe"), 9007199254740992, "Unsafe integer parsed as long")

    const jsonStr3 = '{"large": 9223372036854775808}'
    const record3 = JSON.parseJsonRecord(jsonStr3, opt)
    arktest.assertEQ(record3.get("large"), 9223372036854775808n, "Large integer exceeding long range parsed as BigInt")

    const jsonStr4 = '{"negative": -9223372036854775808}'
    const record4 = JSON.parseJsonRecord(jsonStr4, opt)
    arktest.assertEQ(record4.get("negative"), -9223372036854775808, "Negative long value parsed as long")

    const jsonStr5 = '{"float": 123.456}'
    const record5 = JSON.parseJsonRecord(jsonStr5, opt)
    arktest.assertEQ(record5.get("float"), 123.456, "Float() value should be double, not affected by option")
}

function testParseBigIntDefaultMode(): void {
    const opt: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.DEFAULT} as jsonx.ParseOptions

    const jsonStr = '{"safe": 9007199254740991}'
    const record = JSON.parseJsonRecord(jsonStr, opt)
    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    arktest.assertEQ(record.get("safe"), 9007199254740991, "Safe integer parsed as long")

    const jsonStr2 = '{"unsafe": 9007199254740992}'
    const record2 = JSON.parseJsonRecord(jsonStr2, opt)
    arktest.assertEQ(record2.get("unsafe"), 9007199254740992, "Unsafe integer parsed as long")

    const jsonStr3 = '{"large": 9223372036854775808}'
    arktest.expectThrow(() => {
        JSON.parseJsonRecord(jsonStr3, opt)
    })

    const jsonStr4 = '{"negative": -9223372036854775808}'
    const record4 = JSON.parseJsonRecord(jsonStr4, opt)
    arktest.assertEQ(record4.get("negative"), -9223372036854775808, "Negative long value parsed as long")

    const jsonStr5 = '{"float": 123.456}'
    const record5 = JSON.parseJsonRecord(jsonStr5, opt)
    arktest.assertEQ(record5.get("float"), 123.456, "Float value should be double, not affected by option")
}

function testParseNestedObjectWithBigInt(): void {
    const opt: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT} as jsonx.ParseOptions

    const jsonStr = '{"user": {"id": 9007199254740992, "balance": 1000000000000000000000}}'
    const record = JSON.parseJsonRecord(jsonStr, opt)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    let user = record.get("user") as Record<string, JsonRecordType>
    arktest.assertNE(user, undefined, "Nested object should exist")
    arktest.assertEQ(user.size, 2, "Nested object should have 2 entries")
    arktest.assertEQ(user.get("id"), 9007199254740992n, "Nested BigInt value correct")
    arktest.assertEQ(user.get("balance"), 1000000000000000000000n, "Nested large BigInt value correct")

    const opt2: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.PARSE_AS_BIGINT} as jsonx.ParseOptions
    const jsonStr2 = '{"user": {"id": 9007199254740991, "amount": 123.456}}'
    const record2 = JSON.parseJsonRecord(jsonStr2, opt2)
    let user2 = record2.get("user") as Record<string, JsonRecordType>
    arktest.assertEQ(user2.get("id"), 9007199254740991, "Nested integer parsed as long")
    arktest.assertEQ(user2.get("amount"), 123.456, "Nested float parsed as double")
}

function testParseArrayWithBigInt(): void {
    const opt: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.ALWAYS_PARSE_AS_BIGINT} as jsonx.ParseOptions

    const jsonStr = '{"numbers": [9007199254740992, 9223372036854775808, -9007199254740992]}'
    const record = JSON.parseJsonRecord(jsonStr, opt)

    arktest.assertEQ(record.size, 1, "Record should have 1 entry")
    let arr = record.get("numbers") as Array<JsonRecordType>
    arktest.assertNE(arr, undefined, "Array should exist")
    arktest.assertEQ(arr.length, 3, "Array should have 3 elements")
    arktest.assertEQ(arr[0], 9007199254740992n, "First BigInt element correct")
    arktest.assertEQ(arr[1], 9223372036854775808n, "Second BigInt element correct")
    arktest.assertEQ(arr[2], -9007199254740992n, "Third BigInt element correct")

    const opt2: jsonx.ParseOptions = {bigIntMode: jsonx.BigIntMode.PARSE_AS_BIGINT} as jsonx.ParseOptions
    const jsonStr2 = '{"values": [123, 456.789, 9007199254740992]}'
    const record2 = JSON.parseJsonRecord(jsonStr2, opt2)
    let arr2 = record2.get("values") as Array<JsonRecordType>
    arktest.assertEQ(arr2.length, 3, "Array should have 3 elements")
    arktest.assertEQ(arr2[0], 123, "Integer element parsed as long")
    arktest.assertEQ(arr2[1], 456.789, "Float element parsed as double")
    arktest.assertEQ(arr2[2], 9007199254740992, "Large integer parsed as long")
}
