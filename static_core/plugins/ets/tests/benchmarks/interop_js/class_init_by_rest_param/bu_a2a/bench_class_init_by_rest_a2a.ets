
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

class UnlimitedArguments {
    a: number = 0;
    constructor(...a: number[]) {
        for (let i = 0; i < a.length; i++) {
            this.a += a[i];
        }

    }
}

/**
 * @State
 * @Tags interop, bu_a2a
 */
class ClassInitByRestA2a {

    /**
     * @Setup
     */
    public setup(): void {
        console.log('Starting');
    }

    /**
     * @Benchmark
     */
    test(): number {
        this.init(this.createArgArray(0));

        this.init(this.createArgArray(5));
    
        this.init(this.createArgArray(10));
    
        this.init(this.createArgArray(20));
    
        this.init(this.createArgArray(50));
    
        this.init(this.createArgArray(100));
    
        this.init(this.createArgArray(200));

        return 1;
    }

    createArgArray(count: number): number[] {
        const arr = new number[count];

        for (let i = 0; i < count; i++) {
            arr[i] = i;
        }

        return arr;
    }

    init(
        arg: number[],
    ): void {
        //NOTE: issue(19331) uncomment this after fix spread operator in constructor
        // new UnlimitedArguments(...arg);
    }
}