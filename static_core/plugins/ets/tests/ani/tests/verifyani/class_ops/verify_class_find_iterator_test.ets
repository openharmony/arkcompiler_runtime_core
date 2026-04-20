/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class TestClass {
    public $_iterator() {
        return new TestIterator(this);
    }
}

class TestIterator implements Iterator<int> {
    public index = 0
    public data: int[] = [1, 2, 3]
    public base: TestClass

    constructor(base: TestClass) {
        this.base = base
    }

    next(): IteratorResult<int> {
        return new IteratorResult<int>(this.index >= this.data.length,
            this.index >= this.data.length ? undefined : this.data[this.index++]);
    }
}

class NoIterator {
    public field: int = 7;
}
