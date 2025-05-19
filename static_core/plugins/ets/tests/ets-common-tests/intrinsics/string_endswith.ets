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
    arktest.assertTrue(Empty.endsWith(''));
    arktest.assertTrue(Empty.endsWith('', -1));
    arktest.assertTrue(Empty.endsWith('', 0));
    arktest.assertTrue(Empty.endsWith('', 1));
    arktest.assertFalse(Empty.endsWith('x'));
    arktest.assertFalse(Empty.endsWith('x', -1));
    arktest.assertFalse(Empty.endsWith('x', 0));
    arktest.assertFalse(Empty.endsWith('x', 1));
    arktest.assertFalse(Empty.endsWith('x', 100));

    arktest.assertTrue(Hello.endsWith(''));
    arktest.assertTrue(Hello.endsWith('',-1));
    arktest.assertTrue(Hello.endsWith('', 0));
    arktest.assertTrue(Hello.endsWith('', 1));
    arktest.assertTrue(Hello.endsWith('', 5));
    arktest.assertTrue(Hello.endsWith('', 100));

    arktest.assertTrue(Hello.endsWith('o'));
    arktest.assertFalse(Hello.endsWith('o', -1));
    arktest.assertFalse(Hello.endsWith('o', 0));
    arktest.assertFalse(Hello.endsWith('o', 1));
    arktest.assertFalse(Hello.endsWith('o', 2));
    arktest.assertFalse(Hello.endsWith('o', 3));
    arktest.assertFalse(Hello.endsWith('o', 4));
    arktest.assertTrue(Hello.endsWith('o', 5));
    arktest.assertTrue(Hello.endsWith('o', 100));

    arktest.assertTrue(Hello.endsWith('ello'));
    arktest.assertFalse(Hello.endsWith('ello', -1));
    arktest.assertFalse(Hello.endsWith('ello', 0));
    arktest.assertFalse(Hello.endsWith('ello', 1));
    arktest.assertFalse(Hello.endsWith('ello', 2));
    arktest.assertFalse(Hello.endsWith('ello', 3));
    arktest.assertFalse(Hello.endsWith('ello', 4));
    arktest.assertTrue(Hello.endsWith('ello', 5));
    arktest.assertTrue(Hello.endsWith('ello', 100));

    arktest.assertTrue(Hello.endsWith('Hello'));
    arktest.assertFalse(Hello.endsWith('Hello', -1));
    arktest.assertFalse(Hello.endsWith('Hello', 0));
    arktest.assertFalse(Hello.endsWith('Hello', 1));
    arktest.assertFalse(Hello.endsWith('Hello', 2));
    arktest.assertFalse(Hello.endsWith('Hello', 3));
    arktest.assertFalse(Hello.endsWith('Hello', 4));
    arktest.assertFalse(Hello.endsWith('Hello', 4.0));
    arktest.assertFalse(Hello.endsWith('Hello', 4.5));
    arktest.assertTrue(Hello.endsWith('Hello', 5));
    arktest.assertTrue(Hello.endsWith('Hello', 5.0));
    arktest.assertTrue(Hello.endsWith('Hello', 5.5));
    arktest.assertTrue(Hello.endsWith('Hello', 100));
    arktest.assertTrue(Hello.endsWith('Hello', 100.5));

    arktest.assertTrue(Nature.endsWith(''));
    arktest.assertTrue(Nature.endsWith('', -1));
    arktest.assertTrue(Nature.endsWith('', 0));
    arktest.assertTrue(Nature.endsWith('', 1));
    arktest.assertTrue(Nature.endsWith('', 2));
    arktest.assertTrue(Nature.endsWith('', 3));
    arktest.assertTrue(Nature.endsWith('', 100));

    arktest.assertTrue(Nature.endsWith('然界'));
    arktest.assertFalse(Nature.endsWith('然界', -1));
    arktest.assertFalse(Nature.endsWith('然界', 0));
    arktest.assertFalse(Nature.endsWith('然界', 1));
    arktest.assertFalse(Nature.endsWith('然界', 2));
    arktest.assertTrue(Nature.endsWith('然界', 3));
    arktest.assertTrue(Nature.endsWith('然界', 3.0));
    arktest.assertTrue(Nature.endsWith('然界', 3.5));
    arktest.assertTrue(Nature.endsWith('然界', 100));
    arktest.assertTrue(Nature.endsWith('然界', 100.5));

    arktest.assertTrue(Nature.endsWith('自然界'));
    arktest.assertFalse(Nature.endsWith('自然界', -1));
    arktest.assertFalse(Nature.endsWith('自然界', 0));
    arktest.assertFalse(Nature.endsWith('自然界', 1));
    arktest.assertFalse(Nature.endsWith('自然界', 2));
    arktest.assertTrue(Nature.endsWith('自然界', 100));

    return 0;
}
