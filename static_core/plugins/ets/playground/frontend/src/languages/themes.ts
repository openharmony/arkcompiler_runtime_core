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

import * as monaco from 'monaco-editor';

const CUSTOM_TOKEN_RULES: monaco.editor.ITokenThemeRule[] = [
    // Block control: BB, start/end/try/catch/loop/head/prehead/depth
    { token: 'keyword.block', foreground: 'C586C0', fontStyle: 'bold' },
    // Flow control opcodes: Return, Call, If, Goto, Throw, Try, While
    { token: 'keyword.flow', foreground: 'C586C0', fontStyle: 'bold' },
    // Arithmetic opcodes: Add, Sub, Mul, Div...
    { token: 'keyword.arithmetic', foreground: '569CD6' },
    // Bitwise opcodes: And, Or, Xor, Shl...
    { token: 'keyword.bitwise', foreground: '4EC9B0' },
    // Comparison opcodes: Cmp, Compare, Select...
    { token: 'keyword.comparison', foreground: 'DCDCAA' },
    // Memory opcodes: Load*, Store*...
    { token: 'keyword.memory', foreground: 'CE9178' },
    // Object opcodes: NewObject, InitClass...
    { token: 'keyword.object', foreground: '4EC9B0' },
    // Cast opcodes: Cast, Bitcast, CheckCast...
    { token: 'keyword.cast', foreground: 'DCDCAA' },
    // Check opcodes: NullCheck, BoundsCheck...
    { token: 'keyword.check', foreground: 'CE9178' },
    // Special opcodes: SafePoint, Parameter, Phi, Constant...
    { token: 'keyword.special', foreground: 'F44747', fontStyle: 'bold' },
    // Intrinsic
    { token: 'keyword.intrinsic', foreground: 'F44747' },
    // Optimization: Deoptimize...
    { token: 'keyword.optimization', foreground: '858585' },
    // Call: Builtin
    { token: 'keyword.call', foreground: 'DCDCAA' },
    // Comparison operators: EQ, NE, LT, GT, LE, GE
    { token: 'keyword.operator', foreground: '569CD6' },
    // Block properties: prop:, hotness=, inlining_depth=, file=, caller=
    { token: 'keyword.property.irdump', foreground: '9CDCFE' },
    // Types: void, i32, ref...
    { token: 'type.irdump', foreground: '569CD6' },
    // Class/namespace: Foo::
    { token: 'type.identifier', foreground: '4EC9B0' },
    // Virtual registers: v0, v1...
    { token: 'variable.irdump', foreground: '9CDCFE' },
    // Physical registers: vr0, vr1...
    { token: 'variable.physical', foreground: '4FC1FF' },
    // Usage list after -> (gray)
    { token: 'variable.usage', foreground: '858585' },
    // x86-64 registers
    { token: 'variable.x64', foreground: '569CD6' },
    // Numbers (number matches vs-dark default, number.hex.irdump is irdump-specific)
    { token: 'number.hex.irdump', foreground: 'B5CEA8' },
    // Method/function names in header
    { token: 'entity.function.irdump', foreground: 'DCDCAA' },
    // Comments
    { token: 'comment.irdump', foreground: '6A9955' },
    // Function/method names in arkasm (scoped to avoid affecting other languages)
    { token: 'function.arkasm', foreground: 'DCDCAA' },
];

const DARK_TO_LIGHT: Record<string, string> = {
    'C586C0': 'AF00DB',
    '569CD6': '0000FF',
    '4EC9B0': '267F99',
    'DCDCAA': '795E26',
    'CE9178': 'A31515',
    'F44747': 'E50000',
    '858585': '808080',
    '9CDCFE': '001080',
    '4FC1FF': '0070C1',
    'B5CEA8': '098658',
    '6A9955': '008000',
};

export const THEME_DARK = 'playground-dark';
export const THEME_LIGHT = 'playground-light';

let themesReady = false;

export const ensureThemes = (): void => {
    if (themesReady) {
        return;
    }
    monaco.editor.defineTheme(THEME_DARK, {
        base: 'vs-dark',
        inherit: true,
        rules: CUSTOM_TOKEN_RULES,
        colors: {},
    });
    monaco.editor.defineTheme(THEME_LIGHT, {
        base: 'vs',
        inherit: true,
        rules: CUSTOM_TOKEN_RULES.map(r => ({
            ...r,
            foreground: DARK_TO_LIGHT[r.foreground ?? ''] ?? r.foreground,
        })),
        colors: {},
    });
    themesReady = true;
};

export const getThemeName = (theme: string): string =>
    theme === 'dark' ? THEME_DARK : THEME_LIGHT;
