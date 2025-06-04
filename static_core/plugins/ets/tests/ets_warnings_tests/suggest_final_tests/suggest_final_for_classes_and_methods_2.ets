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

class I { // No final - inheritance
    fn(): String { // No final - overridden
        return "hehe"
    };
}

class A extends I { // Suggest final
    something(): void {} // Suggest final
    override fn(): String {
        return "";
    }
}

class B {  // Suggest final
    yoo(): String {  // Suggest final
        return "yo";
    }
}

class K {  // Suggest final
    final koko(): void {} // No final - already final
}
