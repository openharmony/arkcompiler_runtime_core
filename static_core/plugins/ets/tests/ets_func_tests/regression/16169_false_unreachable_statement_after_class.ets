/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

 /*---
 desc: Issue 17934.
 name: regression/16169_false_unreachable_statement_after_class.ets
 tags: []

 ---*/


function accessible(): void {
}

class Outer {
    myMethod0(): int {
        class One {
        }
        accessible();
        class Two extends One {
        }
        return 0;
    }

    myMethod1(): int {
        interface One {
        }
        accessible();
        class Two implements One {
        }
        return 0;
    }
    myMethod2(): int {
        abstract class One {
        }
        accessible();
        class Two extends One {
        }
        return 0;
    }
}



function return0(): int {
    class One {
    }
    accessible();
    class Two extends One {
    }
    return 0;
}

function return1(): int {
    interface One {
    }
    accessible();
    class Two implements One {
    }
    return 0;
}

function return2(): int {
    class One {
    }
    accessible();
    class Two extends One {
    }
    return 0;
}

function main(): int {
    return 0;
}
