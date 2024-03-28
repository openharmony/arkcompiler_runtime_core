/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

package std.core;

export interface Function<R> {}

export interface Function0<out R> extends Function<R> {
    invoke0(): R;
}

export interface Function1<in P1, out R> extends Function<R> {
    invoke0(p1: P1): R;
}

export interface Function2<in P1, in P2, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2): R;
}

export interface Function3<in P1, in P2, in P3, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3): R;
}

export interface Function4<in P1, in P2, in P3, in P4, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4): R;
}

export interface Function5<in P1, in P2, in P3, in P4, in P5, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5): R;
}

export interface Function6<in P1, in P2, in P3, in P4, in P5, in P6, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6): R;
}

export interface Function7<in P1, in P2, in P3, in P4, in P5, in P6, in P7, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6, p7: P7): R;
}

export interface Function8<in P1, in P2, in P3, in P4, in P5, in P6, in P7, in P8, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6, p7: P7, p8: P8): R;
}

export interface Function9<in P1, in P2, in P3, in P4, in P5, in P6, in P7, in P8, in P9, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6, p7: P7, p8: P8, p9: P9): R;
}

export interface Function10<in P1, in P2, in P3, in P4, in P5, in P6, in P7, in P8, in P9, in P10, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6, p7: P7, p8: P8, p9: P9, p10: P10): R;
}

export interface Function11<in P1, in P2, in P3, in P4, in P5, in P6, in P7, in P8, in P9, in P10, in P11, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6, p7: P7, p8: P8, p9: P9, p10: P10, p11: P11): R;
}

export interface Function12<in P1, in P2, in P3, in P4, in P5, in P6, in P7, in P8, in P9, in P10, in P11, in P12, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6, p7: P7, p8: P8, p9: P9, p10: P10, p11: P11, p12: P12): R;
}

export interface Function13<in P1, in P2, in P3, in P4, in P5, in P6, in P7, in P8, in P9, in P10, in P11, in P12, in P13, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6, p7: P7, p8: P8, p9: P9, p10: P10, p11: P11, p12: P12, p13: P13): R;
}

export interface Function14<in P1, in P2, in P3, in P4, in P5, in P6, in P7, in P8, in P9, in P10, in P11, in P12, in P13, in P14, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6, p7: P7, p8: P8, p9: P9, p10: P10, p11: P11, p12: P12, p13: P13, p14: P14): R;
}

export interface Function15<in P1, in P2, in P3, in P4, in P5, in P6, in P7, in P8, in P9, in P10, in P11, in P12, in P13, in P14, in P15, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6, p7: P7, p8: P8, p9: P9, p10: P10, p11: P11, p12: P12, p13: P13, p14: P14, p15: P15): R;
}

export interface Function16<in P1, in P2, in P3, in P4, in P5, in P6, in P7, in P8, in P9, in P10, in P11, in P12, in P13, in P14, in P15, in P16, out R> extends Function<R> {
    invoke0(p1: P1, p2: P2, p3: P3, p4: P4, p5: P5, p6: P6, p7: P7, p8: P8, p9: P9, p10: P10, p11: P11, p12: P12, p13: P13, p14: P14, p15: P15, p16: P16): R;
}

export interface FunctionN<R> extends Function<R> {
    invoke0(...p: (Object|null|undefined)[]): R;
}
