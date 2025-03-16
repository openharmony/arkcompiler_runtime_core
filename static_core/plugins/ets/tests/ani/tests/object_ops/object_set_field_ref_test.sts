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

class Boxx {
    int_value: int = 0;
    string_value: String = "";
}

class Boxc extends Boxx {
    str_value: String = "";
}

function newBoxObject() {
    return new Boxx();
}

function newBoxcObject() {
    return new Boxc();
}

function checkStringValue(b: Boxx, value: String): boolean {
    return b.string_value == value;
}

function checkStrValue(b: Boxc, value: String): boolean {
    return b.str_value == value;
}
