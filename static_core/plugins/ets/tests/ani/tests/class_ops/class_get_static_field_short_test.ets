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
class TestShort {
    static short_value: short = 3;
    static string_value: String = "testString";
    static special2: string = "";
    static special3: null = null;
    static special4: undefined  = undefined ;
    static special5: int = 0;
    static shortMin: short = -32768;
    static shortMax: short = 32767;
}

class TestShortA extends TestShort {
    static short_value: short = 3;
}

final class TestShortFinal extends TestShort {
    static short_value: short = 3;
}