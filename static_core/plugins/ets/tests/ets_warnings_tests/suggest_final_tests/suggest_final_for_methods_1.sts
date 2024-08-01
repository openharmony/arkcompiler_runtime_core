/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

class I { // No final - inheritance
    aaa(): String { // No final - overrided
        return "a"
    };
}

final class A extends I { // No final - is already final
    override aaa(): String { // No final - methods of final class are already final
        return "";
    }
}

function main(): int { // No final - ETSGLBAL
    return 0;
}
