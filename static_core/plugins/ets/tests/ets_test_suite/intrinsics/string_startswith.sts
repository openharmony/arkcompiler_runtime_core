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

const Empty: String = '';
const Esc: String = '\t\n\v';
const Hello: String = 'Hello';
const Nature: String = '自然界';

function main(): int {

    assertTrue(Empty.startsWith(''));
    assertTrue(Empty.startsWith('', -1));
    assertTrue(Empty.startsWith('', 0));
    assertTrue(Empty.startsWith('', 1));
    assertFalse(Empty.startsWith('*'))
    assertFalse(Empty.startsWith('*', -1))
    assertFalse(Empty.startsWith('*', 0))
    assertFalse(Empty.startsWith('*', 1))

    assertTrue(Esc.startsWith('\t'))
    assertTrue(Esc.startsWith('\t', -1))
    assertTrue(Esc.startsWith('\t', 0))
    assertTrue(Esc.startsWith('\n', 1))
    assertTrue(Esc.startsWith('\v', 2))

    assertTrue(Hello.startsWith(''))
    assertTrue(Hello.startsWith('',-1))
    assertTrue(Hello.startsWith('', 0))
    assertTrue(Hello.startsWith('', 1))
    assertTrue(Hello.startsWith('', 2))
    assertTrue(Hello.startsWith('', 3))
    assertTrue(Hello.startsWith('', 4))
    assertTrue(Hello.startsWith('', 5))

    assertTrue(Hello.startsWith('H'))
    assertTrue(Hello.startsWith("Hel"))
    assertTrue(Hello.startsWith("Hel", 0))
    assertTrue(Hello.startsWith("Hel", -1))
    assertTrue(Hello.startsWith("ello", 1))
    assertTrue(Hello.startsWith("o", 4))

    assertTrue(Nature.startsWith('自然界'))
    assertTrue(Nature.startsWith('自然'))
    assertTrue(Nature.startsWith("自"))
    assertTrue(Nature.startsWith("自", -1))
    assertFalse(Nature.startsWith("然", 0))
    assertTrue(Nature.startsWith("然界", 1))
    assertTrue(Nature.startsWith("界", 2))
    assertFalse(Nature.startsWith("界", 3))

    return 0;
}
