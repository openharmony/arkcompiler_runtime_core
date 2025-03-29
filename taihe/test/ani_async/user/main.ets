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
import * as async_test from "../author_generated/async_test";
loadLibrary("ani_async");

function main() {
    console.log("main ets")
    // In function addSync, we set it to throw an exception if the first
    // parameter is equal to 0.
    console.log("in main ets: ", async_test.addSync(1, 2))

    async_test.add(10, 20, (error: Error, data?: int) => {
        if (!error.message) {
            console.log("main finsih test async success ", data);
        } else {
            console.log("main finsih test async ERROR ", error);
        }
    })

    try {
        let retPromise = await async_test.add(0, 2);
        console.log("main finsih test async promise success ")
    } catch (error) {
        console.error("main finsih test async promise ERROR ", error)
    }

    let ibase = async_test.getIBase();
    console.log(ibase.get());

    ibase.setAsync("interface async set", (error: Error) => {
        if (!error.message) {
            console.log("main finsih test interface async success ");
        } else {
            console.log("main finsih test interface async ERROR ", error);
        }
    })

    let p_ibase = ibase.getPromise();
    p_ibase
        .then((ret: NullishType) => {
            let retInner = ret as string;
            console.log(
                "main finsih test interface async success p_ibase ", retInner)
        })
        .catch((ret: NullishType) => {
            let retError = ret as Error;
            console.error(
                "main finsih test interface async promise ERROR p_ibase ",
                retError)
        });

    async_test.IBase.printAsync((error: Error) => {
        if (!error.message) {
            console.log("main finsih test printAsync success ");
        } else {
            console.log("main finsih test printAsync ERROR ", error);
        }
    })

    async_test.getIBaseAsync((error: Error, data?: async_test.IBase) => {
        if (!error.message) {
            console.log("main finsih test async success ", data!.get());
        } else {
            console.log("main finsih test async ERROR ", error);
        }
    })

    try {
        let retPromise = await async_test.getIBasePromise();
        console.log("main finsih test async promise success ")
    } catch (error) {
        console.error("main finsih test async promise ERROR ", error)
    }

    let data: async_test.Data = {a: "abc", b: "efg", c: 3};
    async_test.fromStruct(data, (error: Error) => {
        if (!error.message) {
            console.log("main finsih test async success ");
        } else {
            console.log("main finsih test async ERROR ", error);
        }
    })

    try {
        let retPromise = await async_test.fromStruct(data);
        console.log("main finsih test async promise success ")
    } catch (error) {
        console.error("main finsih test async promise ERROR ", error)
    }

    async_test.toStruct(
        "aaa", "bbb", 1, (error: Error, data?: async_test.Data) => {
            if (!error.message) {
                console.log("main finsih test async success ", data);
            } else {
                console.log("main finsih test async ERROR ", error);
            }
        })

    // In function toStruct, we set it to throw an exception if the third
    // parameter is equal to 0.
    try {
        let retPromise = await async_test.toStruct("aaa", "bbb", 0);
        console.log("main finsih test async promise success ")
    } catch (error) {
        console.error("main finsih test async promise ERROR ", error)
    }

    let p_struct = async_test.toStruct("aaa", "bbb", 1);
    p_struct
        .then((ret: NullishType) => {
            let retInner = ret as async_test.Data;
            console.log("main finsih test async success p_struct ", retInner)
        })
        .catch((ret: NullishType) => {
            let retError = ret as Error;
            console.error(
                "main finsih test async promise ERROR  p_struct ", retError)
        });
}
