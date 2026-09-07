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
 * Structural change, methods: `Sub` gains `tag()`, which it did not declare. Hot
 * reload refuses this shape (METHOD_ADDED -- the vtable would grow); cold reload
 * must accept it. The base program's call site `s.tag()` is compiled against the
 * OLD `Sub` (no own `tag()`), so dispatching to 'M2' proves the loaded `Sub`
 * really carries the added override.
 *
 * `Greeter` rides along unchanged (a recompiled file carries all its classes, and
 * es2panda must resolve `Sub`'s superclass); `Calc` and `Lazy` stay absent and
 * keep falling through to the original file.
 */

class Greeter {
    public n: int = 0;

    public bump(): int {
        this.n = this.n + 1;
        return this.n;
    }

    public tag(): string {
        return 'V1';
    }
}

class Sub extends Greeter {
    // ADDED method (the base program's Sub declares nothing)
    public tag(): string {
        return 'M2';
    }
}

function main(args: FixedArray<string>): int {
    return 0;
}
