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
 * Partial patch (the incremental-compilation shape): ONLY `Greeter` is carried.
 * `Greeter` must come from the patch; `Calc` and `Lazy` are absent here and must
 * fall through to the original file. `Sub` is absent too -- it is loaded from the
 * original file, but its superclass lookup finds the PATCH `Greeter`, so the
 * inherited `tag()` is 'P2' as well.
 */

class Greeter {
    public n: int = 0;

    public bump(): int {
        this.n = this.n + 1;
        return this.n;
    }

    public tag(): string {
        return 'P2';
    }
}

function main(args: FixedArray<string>): int {
    return 0;
}
