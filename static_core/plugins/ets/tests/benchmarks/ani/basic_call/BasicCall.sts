/**
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

/**
 * @State
 * @Tags ani, native
 */
class BasicCall {
    static {
        loadLibrary("basic_call");
    }
    static STRING: string = new String("string");
    static OBJECT: Object = new Object();
    // NOTE (oignatenko) #699 enhance VMB to benchmark native methods directly
    public static native basic_call_s(a: int, b: string, c: long, d: Object): string;
    @ani.unsafe.Direct
    public static native basic_call_sc(a: int, b: int, c: long, d: long): long;
    @ani.unsafe.Quick
    public static native basic_call_sf(a: int, b: string, c: long, d: Object): string;
    public native basic_call_v(a: int, b: string, c: long, d: Object): string;
    @ani.unsafe.Quick
    public native basic_call_vf(a: int, b: string, c: long, d: Object): string;
    public final native basic_call_v_final(a: int, b: string, c: long, d: Object): string;
    @ani.unsafe.Quick
    public final native basic_call_vf_final(a: int, b: string, c: long, d: Object): string;
    public static native basic_call_baseline(): long;

    /**
     * @Setup
     */
    public setup(): void {
        console.log('Starting')
    }

    /**
     * @Benchmark
     */
    public baselineCallBench(): void {
        BasicCall.basic_call_baseline();
    }

    /**
     * @Benchmark
     */
    public staticCallBench(): void {
        BasicCall.basic_call_s(1, BasicCall.STRING, 500000000, BasicCall.OBJECT);
    }

    /**
     * @Benchmark
     */
    public criticalCallBench(): void {
        BasicCall.basic_call_sc(1, 2, 500000000, 5000000001);
    }

    /**
     * @Benchmark
     */
    public fastStaticCallBench(): void {
        BasicCall.basic_call_sf(1, BasicCall.STRING, 500000000, BasicCall.OBJECT);
    }

    /**
     * @Benchmark
     */
    public nonStaticCallBench(): void {
        this.basic_call_v(1, BasicCall.STRING, 500000000, BasicCall.OBJECT);
    }

    /**
     * @Benchmark
     */
    public nonStaticFastCallBench(): void {
        this.basic_call_vf(1, BasicCall.STRING, 500000000, BasicCall.OBJECT);
    }

    /**
     * @Benchmark
     */
    public finalCallBench(): void {
        this.basic_call_v_final(1, BasicCall.STRING, 500000000, BasicCall.OBJECT);
    }

    /**
     * @Benchmark
     */
    public finalFastCallBench(): void {
        this.basic_call_vf_final(1, BasicCall.STRING, 500000000, BasicCall.OBJECT);
    }
}
