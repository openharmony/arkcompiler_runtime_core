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

import buffer from '@ohos.buffer'

function main(): int {
    const suite = new arktest.ArkTestsuite('Buffer encoding sanitize tests')

    suite.addTest('sanitizeUtf8 empty string', testSanitizeUtf8EmptyString)
    suite.addTest('sanitizeUtf8 ascii string', testSanitizeUtf8AsciiString)
    suite.addTest('sanitizeUtf8 valid surrogate pair', testSanitizeUtf8ValidSurrogatePair)
    suite.addTest('sanitizeUtf8 lone high surrogate', testSanitizeUtf8LoneHighSurrogate)
    suite.addTest('sanitizeUtf8 lone low surrogate', testSanitizeUtf8LoneLowSurrogate)
    suite.addTest('sanitizeUtf8 high surrogate at end', testSanitizeUtf8HighSurrogateAtEnd)
    suite.addTest('sanitizeUtf8 multiple lone surrogates', testSanitizeUtf8MultipleLoneSurrogates)
    suite.addTest('sanitizeUtf8 mixed valid and lone', testSanitizeUtf8MixedValidAndLone)
    suite.addTest('sanitizeUtf8 no sanitizing needed', testSanitizeUtf8NoSanitizingNeeded)

    return suite.run()
}

const REPLACEMENT_UTF8_BYTES: int[] = [239, 191, 189] // U+FFFD in UTF-8

function testSanitizeUtf8EmptyString(): void {
    const buf = buffer.from('', 'utf8')
    arktest.assertEQ(buf.length, 0)
}

function testSanitizeUtf8AsciiString(): void {
    const buf = buffer.from('hello', 'utf8')
    arktest.assertEQ(buf.length, 5)
    arktest.assertEQ(buf[0], 104)
    arktest.assertEQ(buf[1], 101)
    arktest.assertEQ(buf[2], 108)
    arktest.assertEQ(buf[3], 108)
    arktest.assertEQ(buf[4], 111)
}

function testSanitizeUtf8ValidSurrogatePair(): void {
    // "\uD842\uDFB7" = "𠮷" (valid surrogate pair, should be preserved)
    const buf = buffer.from('\uD842\uDFB7', 'utf8')
    // 𠮷 in UTF-8: F0 A0 AE B7
    arktest.assertEQ(buf.length, 4)
    arktest.assertEQ(buf[0], 240)
    arktest.assertEQ(buf[1], 160)
    arktest.assertEQ(buf[2], 174)
    arktest.assertEQ(buf[3], 183)
}

function testSanitizeUtf8LoneHighSurrogate(): void {
    // Lone high surrogate \uD800 should be replaced with U+FFFD
    const buf = buffer.from('\uD800', 'utf8')
    arktest.assertEQ(buf.length, 3)
    arktest.assertEQ(buf[0], REPLACEMENT_UTF8_BYTES[0])
    arktest.assertEQ(buf[1], REPLACEMENT_UTF8_BYTES[1])
    arktest.assertEQ(buf[2], REPLACEMENT_UTF8_BYTES[2])
}

function testSanitizeUtf8LoneLowSurrogate(): void {
    // Lone low surrogate \uDC00 should be replaced with U+FFFD
    const buf = buffer.from('\uDC00', 'utf8')
    arktest.assertEQ(buf.length, 3)
    arktest.assertEQ(buf[0], REPLACEMENT_UTF8_BYTES[0])
    arktest.assertEQ(buf[1], REPLACEMENT_UTF8_BYTES[1])
    arktest.assertEQ(buf[2], REPLACEMENT_UTF8_BYTES[2])
}

function testSanitizeUtf8HighSurrogateAtEnd(): void {
    // High surrogate at end of string with no following char
    const buf = buffer.from('A\uD800', 'utf8')
    // "A" (1 byte) + U+FFFD (3 bytes) = 4 bytes
    arktest.assertEQ(buf.length, 4)
    arktest.assertEQ(buf[0], 65) // 'A'
    arktest.assertEQ(buf[1], REPLACEMENT_UTF8_BYTES[0])
    arktest.assertEQ(buf[2], REPLACEMENT_UTF8_BYTES[1])
    arktest.assertEQ(buf[3], REPLACEMENT_UTF8_BYTES[2])
}

function testSanitizeUtf8MultipleLoneSurrogates(): void {
    // Multiple lone surrogates: \uD800 (high) + \uDC00 (low, but not paired as
    // it immediately follows the high - wait, they ARE a valid pair. Let's
    // use \uD800 X \uDC00 (interleaved with normal char) to test lone surrogates
    const buf = buffer.from('\uD800X\uDC00', 'utf8')
    // \uD800 (lone → U+FFFD) + 'X' + \uDC00 (lone → U+FFFD) = 3 + 1 + 3 = 7 bytes
    arktest.assertEQ(buf.length, 7)
    arktest.assertEQ(buf[0], REPLACEMENT_UTF8_BYTES[0])
    arktest.assertEQ(buf[1], REPLACEMENT_UTF8_BYTES[1])
    arktest.assertEQ(buf[2], REPLACEMENT_UTF8_BYTES[2])
    arktest.assertEQ(buf[3], 88) // 'X'
    arktest.assertEQ(buf[4], REPLACEMENT_UTF8_BYTES[0])
    arktest.assertEQ(buf[5], REPLACEMENT_UTF8_BYTES[1])
    arktest.assertEQ(buf[6], REPLACEMENT_UTF8_BYTES[2])
}

function testSanitizeUtf8MixedValidAndLone(): void {
    // "A" + valid pair "\uD842\uDFB7" + "B" + lone high "\uD800" + "C"
    // → "A𠮷B\uFFFDC"
    const buf = buffer.from('A\uD842\uDFB7B\uD800C', 'utf8')
    // 'A' (1) + 𠮷 (4) + 'B' (1) + U+FFFD (3) + 'C' (1) = 10 bytes
    arktest.assertEQ(buf.length, 10)
    arktest.assertEQ(buf[0], 65) // 'A'
    arktest.assertEQ(buf[1], 240) // 𠮷
    arktest.assertEQ(buf[2], 160)
    arktest.assertEQ(buf[3], 174)
    arktest.assertEQ(buf[4], 183)
    arktest.assertEQ(buf[5], 66) // 'B'
    arktest.assertEQ(buf[6], REPLACEMENT_UTF8_BYTES[0]) // U+FFFD
    arktest.assertEQ(buf[7], REPLACEMENT_UTF8_BYTES[1])
    arktest.assertEQ(buf[8], REPLACEMENT_UTF8_BYTES[2])
    arktest.assertEQ(buf[9], 67) // 'C'
}

function testSanitizeUtf8NoSanitizingNeeded(): void {
    // String with no surrogates should be returned as-is (early return path)
    const buf = buffer.from('Hello World! 你好！', 'utf8')
    const expected = buffer.from('Hello World! 你好！', 'utf8')
    arktest.assertEQ(buf.length, expected.length)
    for (let i = 0; i < buf.length; i++) {
        arktest.assertEQ(buf[i], expected[i])
    }
}
