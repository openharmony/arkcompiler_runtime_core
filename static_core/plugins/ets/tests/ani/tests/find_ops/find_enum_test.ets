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

enum Color {
    RED,
    GREEN,
    BLUE
}

enum ColorInt {
    REDINT = 5,
    GREENINT = 77,
    BLUEINT = 999
}

enum ColorString {
    REDSTR = "str_red",
    GREENSTR = "str_green",
    BLUESTR = "str_blue"
}

enum EnumA001 {
    RED,
    GREEN,
    BLUE
}

namespace test003A {
    namespace test003B {
        enum EnumA003 {
            RED,
            GREEN,
            BLUE
        }
    }
}

namespace test004A {
    namespace test004B {
        enum EnumA004 {
            BLUE,
            RED,
            GREEN
        }
    }
    enum EnumA004 {
        RED,
        GREEN,
        BLUE
    }
}