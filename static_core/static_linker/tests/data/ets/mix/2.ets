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

export @interface FieldAnno {
    names: String[] = ["name01", "name02"]
}

export @interface MethodAnno {
    names: String[] = ["name01", "name02"]
}

export @interface ClassAnno {
    names: String[] = ["name01", "name02"]
}

export abstract class Base {
    abstract getName(): String;
}

@ClassAnno({names: ["test"]})
export class Cls extends Base {
    @FieldAnno({names: ["test"]})
    name: string = ""

    setName(name: string) {
        this.name = name
    }
    getName(): string {
        return this.name
    }

    calculate(a: int, b: int): int {
        if(a > b) {
            try {
                return a - b;
            } catch (e) {
                console.log('err')
                return a + b;
            }
        } else {
            return a * b;
        }
    }

    @MethodAnno({names: ["test"]})
    add(a: int, b: int): int {
        return a + b;
    }
}
