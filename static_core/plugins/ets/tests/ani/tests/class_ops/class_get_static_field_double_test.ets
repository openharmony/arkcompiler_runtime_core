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
class TestDouble {
    static double_value: double = 18.0;
    static string_value: String = "18.0";
    static special1: int = 0;
    static special2: string = "";
    static special3: null = null;
    static special4: undefined  = undefined ;
    static doubleMin: double = -1.7976931348623157e+308;
    static minpositive: double = 2.2250738585072014e-308;
    static doubleMax: double = 1.7976931348623157e+308;
}

class TestDoubleA extends TestDouble {
    static double_value: double = 18.0;
}

final class TestDoubleFinal extends TestDouble {
    static double_value: double = 18.0;
}