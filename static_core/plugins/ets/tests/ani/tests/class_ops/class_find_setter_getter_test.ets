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

interface Style {
    color : string
}

class StyledRectangle implements Style {
    color: string = ""
}

class ExplicitMethods {
    private age_: number = 0
    set age(x: number) {
    }
    get age(): number {
        return this.age_
    }
}

class ImplicitMethods {
    field: number
}

class OnlyGet {
    public field_: boolean = true
    get field(): boolean {
        return this.field_
    }
}

class OnlySet {
    public field_: boolean = true
    set field(x: number) {
    }
}
