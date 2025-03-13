/**
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


export class Methods {
    IsTrue(a: boolean): boolean {
        return a;
    }

    SumString(a: string, b: string): string {
        return a + b;
    }

    static StaticSumDouble(a: double, b: double): double {
        return a + b;
    }

    SumIntArray(a: int[]): int {
        let res: int = 0;
        for (let i = 0; i < a.length; i++) {
            res += a[i];
        }
        return res;
    }

    OptionalString(a: string | undefined): string | undefined {
        return a;
    }

    SumIntVariadic(a: int, ...b: int[]): int {
       for (let i = 0; i < b.length; i++) {
           a += b[i];
       }
       return a;
    }
};
