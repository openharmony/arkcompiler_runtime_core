/*
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

//! DEFINE_FRONTEND_OPTIONS DEFAULT ARGS
//! RUN_FRONTEND            options: "--opt-level=1"

//! CHECKER       Test StringBuilder hoisting based on @arkts.math.Decimal.ets code
//! SKIP_IF       @architecture == "arm32"
//! # Check no crash in AOT
//! RUN_PAOC      options: "--compiler-regex='.*digitsToString'"
//! RUN           entry: "ets_string_concat_loop_part4.ETSGLOBAL::bugfixMain"

const LOG_BASE: int = 7;

class Utils {
    public static getZeroString(k: double): string {
        let zs: string = '';
        for (; k--;) {
            zs += '0';
        }
        return zs;
    }

    public static digitsToString(d: Array<double>): string {
        let indexOfLastWord: double = d.length - 1;
        let str: string = '';
        let w: double = d[0];
        if (indexOfLastWord > 0) {
            str += w;
            let i: int;
            let k: double;
            let ws: string;
            for (i = 1; i < indexOfLastWord; i++) {
                ws = d[i] + '';
                k = LOG_BASE - ws.length;
                if (k) {
                    str += Utils.getZeroString(k);
                }
                str += ws;
            }
            w = d[i];
            ws = w + '';
            k = LOG_BASE - ws.length;
            if (k) {
                str += Utils.getZeroString(k);
            }
        } else if (w === 0) {
            return '0';
        }
        for (; w % 10 === 0;) {
            w /= 10;
        }
        return str + w;
    }
}

function bugfixMain() {
    arktest.assertEQ(Utils.digitsToString([0.0]), '0');
}
