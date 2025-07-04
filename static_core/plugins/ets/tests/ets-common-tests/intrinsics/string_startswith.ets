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

const gEmpty: String = '';
const gEsc: String = '\t\n\v';
const gHello: String = 'Hello';
const gNature: String = '自然界';

function main(): int {

    arktest.assertTrue(gEmpty.startsWith(''));
    arktest.assertTrue(gEmpty.startsWith('', -1));
    arktest.assertTrue(gEmpty.startsWith('', 0));
    arktest.assertTrue(gEmpty.startsWith('', 1));
    arktest.assertFalse(gEmpty.startsWith('*'))
    arktest.assertFalse(gEmpty.startsWith('*', -1))
    arktest.assertFalse(gEmpty.startsWith('*', 0))
    arktest.assertFalse(gEmpty.startsWith('*', 1))

    arktest.assertTrue(gEsc.startsWith('\t'))
    arktest.assertTrue(gEsc.startsWith('\t', -1))
    arktest.assertTrue(gEsc.startsWith('\t', 0))
    arktest.assertTrue(gEsc.startsWith('\n', 1))
    arktest.assertTrue(gEsc.startsWith('\v', 2))

    arktest.assertTrue(gHello.startsWith(''))
    arktest.assertTrue(gHello.startsWith('', -1))
    arktest.assertTrue(gHello.startsWith('', 0))
    arktest.assertTrue(gHello.startsWith('', 1))
    arktest.assertTrue(gHello.startsWith('', 2))
    arktest.assertTrue(gHello.startsWith('', 3))
    arktest.assertTrue(gHello.startsWith('', 4))
    arktest.assertTrue(gHello.startsWith('', 5))

    arktest.assertTrue(gHello.startsWith('H'))
    arktest.assertTrue(gHello.startsWith('Hel'))
    arktest.assertTrue(gHello.startsWith('Hel', 0))
    arktest.assertTrue(gHello.startsWith('Hel', -1))
    arktest.assertTrue(gHello.startsWith('ello', 1))
    arktest.assertTrue(gHello.startsWith('o', 4))

    arktest.assertTrue(gNature.startsWith('自然界'))
    arktest.assertTrue(gNature.startsWith('自然'))
    arktest.assertTrue(gNature.startsWith('自'))
    arktest.assertTrue(gNature.startsWith('自', -1))
    arktest.assertFalse(gNature.startsWith('然', 0))
    arktest.assertTrue(gNature.startsWith('然界', 1))
    arktest.assertTrue(gNature.startsWith('界', 2))
    arktest.assertFalse(gNature.startsWith('界', 3))

    return 0;
}
