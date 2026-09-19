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

/*
 * Structural change, fields: `Greeter` gains a field `m`. Hot reload refuses this
 * shape outright (FIELD_CHANGED -- object layout would change under live
 * instances); cold reload has no live instances and must accept it. The value of
 * the added field is observable through the same-signature `tag()`, which reads
 * it, so 'F2-7' proves the added field exists AND is initialised by the new `<init>`.
 */

class Greeter {
    public n: int = 0;

    // ADDED field
    public m: int = 7;

    public bump(): int {
        this.n = this.n + 1;
        return this.n;
    }

    public tag(): string {
        return 'F2-' + this.m;
    }
}

function main(args: FixedArray<string>): int {
    return 0;
}
