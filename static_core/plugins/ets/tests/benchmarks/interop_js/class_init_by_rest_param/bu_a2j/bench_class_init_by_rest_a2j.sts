
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

import {
    UnlimitedArguments,
} from 'import';

/**
 * @State
 * @Tags interop, bu_a2j
 */
class ClassInitByRestA2j {

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

    createArgArray(count: int): Array<Int> {
        let arr: Array<Int> = new Array<Int>

        for (let i: Int = 0; i < count; i++) {
            arr.push(i)
        }
        return arr;
    }

    init(arg: Array<Int>): void {
        new UnlimitedArguments(arg);
    }
}