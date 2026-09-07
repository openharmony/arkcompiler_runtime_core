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
 * Structural change, hierarchy: `Greeter` no longer declares `tag()` itself; the
 * method moves to a NEW base class `NewBase` that the patch also carries. Hot
 * reload refuses this shape (HIERARCHY_CHANGED); cold reload must accept it. The
 * old call site `g.tag()` resolves through the new vtable into `NewBase.tag`.
 */

class NewBase {
    public tag(): string {
        return 'B2';
    }
}

class Greeter extends NewBase {
    public n: int = 0;

    public bump(): int {
        this.n = this.n + 1;
        return this.n;
    }

    // `tag()` is intentionally NOT declared here anymore
}

function main(args: FixedArray<string>): int {
    return 0;
}
