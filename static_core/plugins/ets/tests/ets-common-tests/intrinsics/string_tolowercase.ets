/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

function testToLowerCase() {
    // Test cases for LineString
    const valiedUTF16String: String = new String('Hello 你好 नमस्ते 😊👩💻\uD83D\uDC68\u200D\uD83D\uDCBBАаБбВвГгДдЕеЁё');
    arktest.assertTrue(valiedUTF16String.toLowerCase() === "hello 你好 नमस्ते 😊👩💻👨‍💻ааббввггддееёё", "valid Utf16Chars");
    const validEngChars: String = new String('ALPHABETalphabetABCabc');
    arktest.assertTrue(validEngChars.toLowerCase() === "alphabetalphabetabcabc", "valid EngChars");
    const invalidUTF16String: String = new String('Normal start\uD83DMid content\uD83D\uDC68end');
    arktest.assertTrue(invalidUTF16String.toLowerCase() === "normal start\uD83Dmid content\uD83D\uDC68end", "invalid UTF16Chars");
    
    // Test cases for TreeString
    const validUTF16TreeString: String = new String('Hello 你好 नमस्ते 😊👩💻').concat('😊👩💻ABCabc')   // Multilingual + Emoji
    arktest.assertTrue(validUTF16TreeString.toLowerCase() === "hello 你好 नमस्ते 😊👩💻😊👩💻abcabc", "valid UTF-16 TreeString");
    const validEngTreeString: String = new String('ALPHABETalphabetABCabc').concat('ABCabc');
    arktest.assertTrue(validEngTreeString.toLowerCase() === "alphabetalphabetabcabcabcabc", "valid EngChars TreeString");
    const invalidUTF16TreeString: String = new String('Normal start\uD83DMid content\uD83D\uDC68end').concat('ABCabc');
    arktest.assertTrue(invalidUTF16TreeString.toLowerCase() === "normal start\uD83Dmid content\uD83D\uDC68endabcabc", "invalid UTF16Chars TreeString");

    // Test cases for SlicedString
    const slicedValidUTF16String: String = valiedUTF16String.substring(4, valiedUTF16String.length - 4);
    arktest.assertTrue(slicedValidUTF16String.toLowerCase() === "o 你好 नमस्ते 😊👩💻👨‍💻ааббввггдд", "valid UTF-16 SlicedString");
    const slicedValidEngChars: String = validEngChars.substring(1, validEngChars.length - 1);
    arktest.assertTrue(slicedValidEngChars.toLowerCase() === "lphabetalphabetabcab", "valid EngChars SlicedString");
    const slicedInvalidUTF16String: String = invalidUTF16String.substring(2, invalidUTF16String.length - 2);
    arktest.assertTrue(slicedInvalidUTF16String.toLowerCase() === "rmal start\uD83Dmid content\uD83D\uDC68e", "invalid UTF16Chars SlicedString");

}


function main() {
    let suite = new arktest.ArkTestsuite("StringToLowerCase");
    suite.addTest('testToLowerCase', testToLowerCase);
    return suite.run();
}
