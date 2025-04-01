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

export namespace ops {
    let a: int = 1;
    function getInitialIntValue(): int {
        return 1;
    }
    class A {}
    namespace ttt {}
    enum Color {
        RED,
        BLUE,
        GREEN
    }
    export native function sum(a: int, b: int): int;
}
namespace opst {
    function getInitialIntValue(): int {
        return 0;
    }
}

namespace outer {
    namespace inner {
        let a: int = 1;
        function getInitialIntValue(): int {
            return 1;
        }
        class A {}
        namespace ttt {}
        enum Color {
            RED,
            BLUE,
            GREEN
        }
        export native function sum(a: int, b: int): int;
    }
} 

namespace a {
    namespace a {}
}
namespace b {}
namespace c {}
namespace d {}
namespace e {}
namespace f {}
namespace g {}
namespace h {}
namespace i {}
namespace j {}

namespace at {
    namespace bt {
        namespace ct {}
    }
    namespace ct {}
}
