/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

package escompat;

export class IteratorValue<V> {
    public value: V|null;
    public done: boolean;

    constructor(value: V|null, done: boolean) {
        this.value = value;
        this.done = done;
    }
}

export interface Iterator<V> {
    next(): IteratorValue<V>;
}

export class ValuesIterator<V> implements Iterator<V> {
    private values: V[];
    private pointer: int;

    constructor(values: V[]) {
        if (values != null && values.length > 0) {
            this.values = values;
            this.pointer = 0;
        } else {
            this.pointer = -1;
        }
    }

    public override next(): IteratorValue<V> {
        if (this.pointer == -1) {
            return new IteratorValue<V>(null, true);
        } else {
            let value = this.values[this.pointer];
            this.pointer++;
            if (this.pointer >= this.values.length) {
                this.pointer = -1;
            }
            return new IteratorValue<V>(value, false);
        }
    }
}

export class SetIterator<V> implements Iterator<V> {
    private values: V[];
    private pointer: int;

    constructor(values: V[]) {
        if (values != null && values.length > 0) {
            this.values = values;
            this.pointer = 0;
        } else {
            this.pointer = -1;
        }
    }

    public override next(): IteratorValue<V> {
        if (this.pointer == -1) {
            return new IteratorValue<V>(null, true);
        } else {
            let value = this.values[this.pointer];
            this.pointer++;

            if (this.pointer >= this.values.length) {
                this.pointer = -1;
            }

            return new IteratorValue<V>(value, false);
        }
    }
}

export class MapIterator<K, V> implements Iterator<Entry<K, V>> {
    private values: Entry<K, V>[];
    private pointer: int;

    constructor(values: Entry<K, V>[]) {
        if (values != null && values.length > 0) {
            this.values = values;
            this.pointer = 0;
        } else {
            this.pointer = -1;
        }
    }

    public override next(): IteratorValue<Entry<K, V>> {
        if (this.pointer == -1) {
            return new IteratorValue<Entry<K, V>>(null, true);
        } else {
            let value = this.values[this.pointer];
            this.pointer++;

            if (this.pointer >= this.values.length) {
                this.pointer = -1;
            }

            return new IteratorValue<Entry<K, V>>(value, false);
        }
    }
}
