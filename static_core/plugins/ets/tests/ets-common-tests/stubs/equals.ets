/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

function isEqual<T>(a: T, b: T): boolean {
    return a == b;
}

function erase<T>(v: T): Any {
    return v;
}

enum Color {
    RED,
    GREEN,
    BLUE = 2147483647,
    YELLOW = -2147483648,
}

enum Color64 {
    RED,
    GREEN,
    BLUE = 2147483647,
    YELLOW = -2147483648,
    PURPLE = 9223372036854775807,
    PINK = -9223372036854775808
}

enum ColorString {
    RED = "",
    GREEN = "GREEN"
}

function testNormal() {
    assertTrue(isEqual(Color.RED, Color.RED));
    assertTrue(isEqual(Color.GREEN, Color.GREEN));
    assertTrue(isEqual(Color.BLUE, Color.BLUE));
    assertTrue(isEqual(Color.YELLOW, Color.YELLOW));

    assertTrue(isEqual(Color64.RED, Color64.RED));
    assertTrue(isEqual(Color64.GREEN, Color64.GREEN));
    assertTrue(isEqual(Color64.BLUE, Color64.BLUE));
    assertTrue(isEqual(Color64.YELLOW, Color64.YELLOW));
    assertTrue(isEqual(Color64.PURPLE, Color64.PURPLE));
    assertTrue(isEqual(Color64.PINK, Color64.PINK));

    assertTrue(isEqual(ColorString.RED, ColorString.RED));
    assertTrue(isEqual(ColorString.GREEN, ColorString.GREEN));
    
    assertTrue(!isEqual(Color.RED, Color.GREEN));
    assertTrue(!isEqual(Color.RED, Color.BLUE));
    assertTrue(!isEqual(Color.RED, Color.YELLOW));
    assertTrue(!isEqual(ColorString.RED, ColorString.GREEN));
}

function testErase() {
    assertTrue(isEqual(erase(Color.RED), erase(Color.RED)));
    assertTrue(isEqual(erase(Color.GREEN), erase(Color.GREEN)));
    assertTrue(isEqual(erase(Color.BLUE), erase(Color.BLUE)));
    assertTrue(isEqual(erase(Color.YELLOW), erase(Color.YELLOW)));

    assertTrue(isEqual(erase(Color64.RED), erase(Color64.RED)));
    assertTrue(isEqual(erase(Color64.GREEN), erase(Color64.GREEN)));
    assertTrue(isEqual(erase(Color64.BLUE), erase(Color64.BLUE)));
    assertTrue(isEqual(erase(Color64.YELLOW), erase(Color64.YELLOW)));
    assertTrue(isEqual(erase(Color64.PURPLE), erase(Color64.PURPLE)));
    assertTrue(isEqual(erase(Color64.PINK), erase(Color64.PINK)));

    assertTrue(isEqual(erase(Color64.RED), erase(Color.RED)));
    assertTrue(isEqual(erase(Color64.GREEN), erase(Color.GREEN)));
    assertTrue(isEqual(erase(Color64.BLUE), erase(Color.BLUE)));
    assertTrue(isEqual(erase(Color64.YELLOW), erase(Color.YELLOW)));

    assertTrue(isEqual(erase(ColorString.RED), erase(ColorString.RED)));
    assertTrue(isEqual(erase(ColorString.GREEN), erase(ColorString.GREEN)));

    assertTrue(!isEqual(erase(Color.RED), erase(Color.GREEN)));
    assertTrue(!isEqual(erase(Color.RED), erase(Color.BLUE)));
    assertTrue(!isEqual(erase(Color.RED), erase(Color.YELLOW)));
    assertTrue(!isEqual(erase(ColorString.RED), erase(ColorString.GREEN)));
    assertTrue(!isEqual(erase(Color.BLUE), erase(Color64.PURPLE)));
    assertTrue(!isEqual(erase(Color.YELLOW), erase(Color64.PINK)));
}

function main() {
    testNormal();
    testErase();
}