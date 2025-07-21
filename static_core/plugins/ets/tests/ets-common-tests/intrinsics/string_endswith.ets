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
const Hello: String = 'Hello';
const Nature: String = '自然界';

function main(): int {
    assertTrue(Empty.endsWith(''));
    assertTrue(Empty.endsWith('', -1));
    assertTrue(Empty.endsWith('', 0));
    assertTrue(Empty.endsWith('', 1));
    assertFalse(Empty.endsWith('x'));
    assertFalse(Empty.endsWith('x', -1));
    assertFalse(Empty.endsWith('x', 0));
    assertFalse(Empty.endsWith('x', 1));
    assertFalse(Empty.endsWith('x', 100));

    assertTrue(Hello.endsWith(''));
    assertTrue(Hello.endsWith('',-1));
    assertTrue(Hello.endsWith('', 0));
    assertTrue(Hello.endsWith('', 1));
    assertTrue(Hello.endsWith('', 5));
    assertTrue(Hello.endsWith('', 100));

    assertTrue(Hello.endsWith('o'));
    assertFalse(Hello.endsWith('o', -1));
    assertFalse(Hello.endsWith('o', 0));
    assertFalse(Hello.endsWith('o', 1));
    assertFalse(Hello.endsWith('o', 2));
    assertFalse(Hello.endsWith('o', 3));
    assertFalse(Hello.endsWith('o', 4));
    assertTrue(Hello.endsWith('o', 5));
    assertTrue(Hello.endsWith('o', 100));

    assertTrue(Hello.endsWith('ello'));
    assertFalse(Hello.endsWith('ello', -1));
    assertFalse(Hello.endsWith('ello', 0));
    assertFalse(Hello.endsWith('ello', 1));
    assertFalse(Hello.endsWith('ello', 2));
    assertFalse(Hello.endsWith('ello', 3));
    assertFalse(Hello.endsWith('ello', 4));
    assertTrue(Hello.endsWith('ello', 5));
    assertTrue(Hello.endsWith('ello', 100));

    assertTrue(Hello.endsWith('Hello'));
    assertFalse(Hello.endsWith('Hello', -1));
    assertFalse(Hello.endsWith('Hello', 0));
    assertFalse(Hello.endsWith('Hello', 1));
    assertFalse(Hello.endsWith('Hello', 2));
    assertFalse(Hello.endsWith('Hello', 3));
    assertFalse(Hello.endsWith('Hello', 4));
    assertFalse(Hello.endsWith('Hello', 4.0));
    assertFalse(Hello.endsWith('Hello', 4.5));
    assertTrue(Hello.endsWith('Hello', 5));
    assertTrue(Hello.endsWith('Hello', 5.0));
    assertTrue(Hello.endsWith('Hello', 5.5));
    assertTrue(Hello.endsWith('Hello', 100));
    assertTrue(Hello.endsWith('Hello', 100.5));

    assertTrue(Nature.endsWith(''));
    assertTrue(Nature.endsWith('', -1));
    assertTrue(Nature.endsWith('', 0));
    assertTrue(Nature.endsWith('', 1));
    assertTrue(Nature.endsWith('', 2));
    assertTrue(Nature.endsWith('', 3));
    assertTrue(Nature.endsWith('', 100));

    assertTrue(Nature.endsWith('然界'));
    assertFalse(Nature.endsWith('然界', -1));
    assertFalse(Nature.endsWith('然界', 0));
    assertFalse(Nature.endsWith('然界', 1));
    assertFalse(Nature.endsWith('然界', 2));
    assertTrue(Nature.endsWith('然界', 3));
    assertTrue(Nature.endsWith('然界', 3.0));
    assertTrue(Nature.endsWith('然界', 3.5));
    assertTrue(Nature.endsWith('然界', 100));
    assertTrue(Nature.endsWith('然界', 100.5));

    assertTrue(Nature.endsWith('自然界'));
    assertFalse(Nature.endsWith('自然界', -1));
    assertFalse(Nature.endsWith('自然界', 0));
    assertFalse(Nature.endsWith('自然界', 1));
    assertFalse(Nature.endsWith('自然界', 2));
    assertTrue(Nature.endsWith('自然界', 100));

    return 0;
}
