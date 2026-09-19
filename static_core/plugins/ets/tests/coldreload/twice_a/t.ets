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
 * First of the two double-prepend probes. The base program calls coldReload twice,
 * with this abc first and `twice_b` second; every prepend goes to the FRONT of
 * abcFiles, so `twice_b` must win (see run_smoke.sh, case `twice`). This file is
 * compiled on its own and never run directly.
 */

class Greeter {
    public n: int = 0;

    public bump(): int {
        this.n = this.n + 1;
        return this.n;
    }

    public tag(): string {
        return 'PA';
    }
}

function main(args: FixedArray<string>): int {
    return 0;
}
