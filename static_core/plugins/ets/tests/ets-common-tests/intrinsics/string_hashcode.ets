/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

let compressableStringsTable: FixedArray<String> = [
    '\0',
    '0',
    '01',
    '012',
    '0123',
    '01234',
    '012345',
    '0123456',
    '01234567',
    '012345678',
    '0123456789',
    '0123456789a',
    '0123456789ab',
    '0123456789abc',
    '0123456789abcd',
    '0123456789abcde',
    '0123456789abcdef',
    '0123456789abcdefg',
    '0123456789abcdefgh',
    '0123456789abcdefghi',
    '0123456789abcdefghij',
    '0123456789abcdefghijk'
];

let uncompressableStringsTable: FixedArray<String> = [
    '\u0000',
    '\u00b0',
    '\u00b0\u00b1',
    '\u00b0\u00b1\u00b2',
    '\u00b0\u00b1\u00b2\u00b3',
    '\u00b0\u00b1\u00b2\u00b3\u00b4',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc\u00bd',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc\u00bd\u00be',
    '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc\u00bd\u00be\u00bf'
];

let compressable_hashcode_table: FixedArray<int> = [
    0,
    48,
    1537,
    47697,
    1478658,
    45838450,
    1420992003,
    1101079187,
    -226283516,
    1575145652,
    1584875013,
    1886485244,
    -1648499482,
    436123709,
    634933191,
    -1791907458,
    285443752,
    258821823,
    -566457975,
    -380327936,
    1094735978,
    -422922943
];

let uncompressable_hashcode_table: FixedArray<int> = [
    0,
    176,
    5633,
    174801,
    5419010,
    167989490,
    912707075,
    -1770851565,
    938176516,
    -981298892,
    -355494395,
    1864575829,
    1967276038,
    856015222,
    766668295,
    -2003086441,
    -1966137336
];

let hashCollisionA: FixedArray<char> = [1, 0];
let hashCollisionB: FixedArray<char> = [0, 31];
let hashCollisionC: FixedArray<char> = [2, 0];
let hashCollisionD: FixedArray<char> = [1, 31];

function main(): int {
    for (let i: int = 0; i < compressableStringsTable.length; i++) {
        let hashcode: int = compressableStringsTable[i].$_hashCode();
        if (hashcode != compressable_hashcode_table[i]) {
            console.println('Expected hashcode: ' + compressable_hashcode_table[i] +
                            ', computed hashcode: ' + hashcode +
                            ' (compressable, i = ' + i + ')');
            console.println('FAIL');
            return 1;
        }
    }

    for (let i: int = 0; i < uncompressableStringsTable.length; i++) {
        let hashcode: int = uncompressableStringsTable[i].$_hashCode();
        if (hashcode != uncompressable_hashcode_table[i]) {
            console.println('Expected hashcode: ' + uncompressable_hashcode_table[i] +
                            ', computed hashcode: ' + hashcode +
                            ' (uncompressable, i = ' + i + ')');
            console.println('FAIL');
            return 2;
        }
    }

    // Test hash collisions
    let hcA: String = new String(hashCollisionA);
    let hcB: String = new String(hashCollisionB);
    if (hcA.$_hashCode() != 31) {
      console.println('hcA hashcode: ' + hcA.$_hashCode());
      console.println('FAIL');
      return 3;
    }
    if (hcA.$_hashCode() != hcB.$_hashCode()) {
      console.println('hcA hashcode: ' + hcA.$_hashCode());
      console.println('hcB hashcode: ' + hcB.$_hashCode());
      console.println('FAIL');
      return 4;
    }

    let hcC: String = new String(hashCollisionC);
    let hcD: String = new String(hashCollisionD);
    if (hcC.$_hashCode() != 62) {
      console.println('hcC hashcode: ' + hcC.$_hashCode());
      console.println('FAIL');
      return 5;
    }
    if (hcC.$_hashCode() != hcD.$_hashCode()) {
      console.println('hcC hashcode: ' + hcC.$_hashCode());
      console.println('hcD hashcode: ' + hcD.$_hashCode());
      console.println('FAIL');
      return 6;
    }

    return 0;
}
