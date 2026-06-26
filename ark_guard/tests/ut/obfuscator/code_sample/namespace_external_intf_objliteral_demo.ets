/**
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

/**
 * Namespace that owns a class which constructs an ObjectLiteral whose host
 * is an EXTERNAL interface (std.core.Intl.DateTimeFormatOptions). This
 * exercises the namespace-side refresh chain together with
 * NamespaceRefreshOwnedObjectLiterals: when the namespace renames, any
 * un-refreshed OL whose flattened host name still encodes the old namespace
 * path must be picked up and re-emitted with the new namespace prefix.
 *
 * The test also has a nested class inside the namespace so the same module's
 * recordTable contains both `<mod>.<Ns>.Holder` and the std-core hosted OL,
 * stressing the joint module + namespace refresh paths.
 */

namespace Ns {
    export class Holder {
        public opts: Intl.DateTimeFormatOptions;
        constructor() {
            this.opts = {
                dateStyle: 'short',
            };
        }
    }
}

function main(): void {
    let h = new Ns.Holder();
    let date = new Date();
    let fmt = new Intl.DateTimeFormat('en-US', h.opts);
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('OK', fmt.format(date).length > 0);
}
