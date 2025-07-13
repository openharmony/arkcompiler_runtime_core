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

function testLineStringHashCode() {
    for (let i: int = 0; i < compressableStringsTable.length; i++) {
        let hashcode: int = compressableStringsTable[i].$_hashCode();
        arktest.assertEQ(hashcode, compressable_hashcode_table[i],
            'Expected hashcode: ' + compressable_hashcode_table[i] +
            ', computed hashcode: ' + hashcode +
            ' (compressable, i = ' + i + ')');

        for (let i: int = 0; i < uncompressableStringsTable.length; i++) {
            let hashcode: int = uncompressableStringsTable[i].$_hashCode();
            arktest.assertEQ(hashcode, uncompressable_hashcode_table[i],
                'Expected hashcode: ' + uncompressable_hashcode_table[i] +
                ', computed hashcode: ' + hashcode +
                ' (uncompressable, i = ' + i + ')');
        }
    }
}

function testTreeStringHashCode() {
    let TreeCompressableStr_0: String = '0123456789abcdef';
    let TreeCompressableStr_1: String = 'ghijk';
    let TreeCompressableStr = TreeCompressableStr_0.concat(TreeCompressableStr_1);
    arktest.assertTrue(TreeCompressableStr === compressableStringsTable[compressableStringsTable.length - 1]);
    arktest.assertEQ(TreeCompressableStr.$_hashCode(), 
         compressable_hashcode_table[compressableStringsTable.length - 1]);

    let TreeUncompressableStr_0: String = '\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7';
    let TreeUncompressableStr_1: String = '\u00b8\u00b9\u00ba\u00bb\u00bc\u00bd\u00be\u00bf';
    let TreeUncompressableStr = TreeUncompressableStr_0.concat(TreeUncompressableStr_1);
    arktest.assertTrue(TreeUncompressableStr === uncompressableStringsTable[uncompressableStringsTable.length - 1]);
    arktest.assertEQ(TreeUncompressableStr.$_hashCode(), 
      uncompressable_hashcode_table[uncompressable_hashcode_table.length - 1]);

}

function testSlicedStringHashCode() {
    let ToSliceCompressableStr: String = '123_0123456789abcdefghijk_456';
    let SliceCompressableStr: String = ToSliceCompressableStr.substring(4, ToSliceCompressableStr.getLength() - 4);
    arktest.assertTrue(SliceCompressableStr === compressableStringsTable[compressableStringsTable.length - 1]);
    arktest.assertEQ(SliceCompressableStr.$_hashCode(), 
       compressable_hashcode_table[compressableStringsTable.length - 1]);

    let ToSliceUncompressableStr: 
      String = '\u00b0\u00b0\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc\u00bd\u00be\u00bf\u00b0\u00b0';
    let SliceUncompressableStr: String = 
      ToSliceUncompressableStr.substring(2, ToSliceUncompressableStr.getLength() - 2);
    arktest.assertTrue(SliceUncompressableStr === uncompressableStringsTable[uncompressableStringsTable.length - 1]);
    arktest.assertEQ(SliceUncompressableStr.$_hashCode(), 
      uncompressable_hashcode_table[uncompressable_hashcode_table.length - 1]);
}

function testHashCollision() {
    let hcA: String = new String(hashCollisionA);
    let hcB: String =  new String(hashCollisionB);
    arktest.assertTrue(hcA.$_hashCode() === 31);
    arktest.assertEQ(hcA.$_hashCode(), hcB.$_hashCode());

    let hcC: String =  new String(hashCollisionC);
    let hcD: String =  new String(hashCollisionD);
    arktest.assertTrue(hcC.$_hashCode() === 62);
    arktest.assertEQ(hcC.$_hashCode(), hcD.$_hashCode());
}

function main(): int {
    let suite = new arktest.ArkTestsuite('StringHashCode');
    suite.addTest('testLineStringHashCode', testLineStringHashCode);
    suite.addTest('testTreeStringHashCode', testTreeStringHashCode);
    suite.addTest('testSlicedStringHashCode', testSlicedStringHashCode);
    suite.addTest('testHashCollision', testHashCollision);
    return suite.run();
}
