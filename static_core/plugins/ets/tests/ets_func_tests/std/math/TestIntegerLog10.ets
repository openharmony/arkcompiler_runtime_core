/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

function main(): int {
    let failures: int = 0;
    failures += test(good(1, 0), "Log10 from int 1");
    failures += test(good(10, 1), "Log10 from int 10");
    failures += test(good(100, 2), "Log10 from int 100");
    failures += test(good(1000, 3), "Log10 from int 1000");
    failures += test(good(10000, 4), "Log10 from int 10000");
    failures += test(good(100000, 5), "Log10 from int 100000");
    failures += test(good(1000000, 6), "Log10 from int 1000000");
    failures += test(good(10000000, 7), "Log10 from int 10000000");
    failures += test(good(100000000, 8), "Log10 from int 100000000");
    failures += test(good(1000000000, 9), "Log10 from int 1000000000");
    
    failures += test(good(5, 0), "Log10 from int 5");
    failures += test(good(15, 1), "Log10 from int 15");
    failures += test(good(146, 2), "Log10 from int 146");
    failures += test(good(1123456789, 9), "Log10 from int 1123456789");

    failures += test(bad(), "Bad value passed")
    return test(failures, "All test run")
}

function test(result: int, message: String ): int {
    if(result == 0) {
      return 0;      
    }
    console.println("FAILED: " + message);
    return 1;
}

function bad():int {
    try {
        log10(-1)
    } catch(aor: ArgumentOutOfRangeException) {        
        return 0;
    } catch(e) {
        return 0;   
    }
    return 1;
}

function good(v: int, expected: int):int {
    try {
        if(log10(v) == expected) return 0;
        return 1;
    } catch(e) {
        return 1;
    }
}
