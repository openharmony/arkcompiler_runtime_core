/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 */
class Sanity {

    /**
     * @Param 10
     */
    var size: Int?
    var s0: Int = 0
    var resource: [Int] = [0, 0, 0, 0, 0]

    /**
     * @Setup
     */
    func FillArray() {
        resource = [46, 44, 21, 37, 84]
    }

    /**
     * @Benchmark
     */
    func test() {
        s0 = 0
        var end = size ?? 100
        for i in 0..<end {
            s0 += resource[i % 5]
        }
    }
}
