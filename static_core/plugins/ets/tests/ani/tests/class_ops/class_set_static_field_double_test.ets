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
class TestSetDouble {
    static double_value: double = 18.0;
    static string_value: string = "";
}

class TestSetDoubleA extends TestSetDouble {
    static double_value: double = 18.0;
}

final class TestSetDoubleFinal extends TestSetDouble {
    static double_value: double = 18.0;
}