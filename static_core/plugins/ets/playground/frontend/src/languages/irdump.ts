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

import type { LanguagePlugin } from './types';

const controlFlow = [
    'Call', 'CallDynamic', 'CallIndirect', 'CallNative',
    'CallResolvedStatic', 'CallResolvedVirtual', 'CallStatic', 'CallVirtual',
    'Else', 'Goto', 'If', 'IfImm',
    'Return', 'ReturnI', 'ReturnInlined', 'ReturnVoid',
    'Throw', 'Try', 'While', 'WhilePhi',
];

const arithmetic = [
    'Abs', 'Add', 'AddI', 'AddOverflow', 'AddOverflowCheck', 'AddSR',
    'Div', 'DivI', 'MAdd', 'MNeg', 'MSub', 'Max', 'Min',
    'Mod', 'ModI', 'Mul', 'MulI', 'MulOverflowCheck',
    'Neg', 'NegOverflowAndZeroCheck', 'NegSR', 'Sqrt',
    'Sub', 'SubI', 'SubOverflow', 'SubOverflowCheck', 'SubSR',
];

const bitwise = [
    'AShr', 'AShrI', 'And', 'AndI', 'AndNot', 'AndNotSR', 'AndSR',
    'ExtractBitfield', 'Not',
    'Or', 'OrI', 'OrNot', 'OrNotSR', 'OrSR',
    'Shl', 'ShlI', 'Shr', 'ShrI',
    'Xor', 'XorI', 'XorNot', 'XorNotSR', 'XorSR',
];

const comparison = [
    'Cmp', 'Compare', 'CompareAnyType',
    'Select', 'SelectImm', 'SelectImmTransform', 'SelectTransform',
];

const memory = [
    'FillConstArray', 'LenArray',
    'Load', 'LoadArray', 'LoadArrayI', 'LoadArrayPair', 'LoadArrayPairI',
    'LoadCompressedStringChar', 'LoadCompressedStringCharI',
    'LoadConstArray', 'LoadConstantPool', 'LoadFromConstantPool',
    'LoadI', 'LoadLexicalEnv', 'LoadNative',
    'LoadObject', 'LoadObjectDynamic', 'LoadObjectPair', 'LoadPairPart',
    'LoadResolvedObjectField', 'LoadResolvedObjectFieldStatic',
    'LoadStatic', 'LoadString',
    'Store', 'StoreArray', 'StoreArrayI', 'StoreArrayPair', 'StoreArrayPairI',
    'StoreI', 'StoreNative',
    'StoreObject', 'StoreObjectDynamic', 'StoreObjectPair',
    'StoreResolvedObjectField', 'StoreResolvedObjectFieldStatic',
    'StoreStatic', 'UnresolvedStoreStatic',
];

const object = [
    'GetGlobalVarAddress', 'GetInstanceClass',
    'InitClass', 'InitEmptyString', 'InitObject', 'InitString',
    'LoadAndInitClass', 'LoadClass', 'LoadImmediate', 'LoadObjFromConst',
    'LoadRuntimeClass', 'LoadType', 'LoadUniqueObject',
    'MultiArray', 'NewArray', 'NewObject',
    'UnresolvedLoadAndInitClass', 'UnresolvedLoadType',
];

const cast = [
    'Bitcast', 'Cast', 'CastAnyTypeValue', 'CastValueToAnyType',
    'CheckCast', 'GetAnyTypeName', 'IsInstance',
];

const check = [
    'AnyTypeCheck', 'BoundsCheck', 'BoundsCheckI', 'HclassCheck',
    'NegativeCheck', 'NotPositiveCheck', 'NullCheck', 'ObjByIndexCheck',
    'RefTypeCheck', 'ResolveByName', 'ResolveObjectField',
    'ResolveObjectFieldStatic', 'ResolveStatic', 'ResolveVirtual',
    'StringFlatCheck', 'ZeroCheck',
];

const special = [
    'CatchPhi', 'Constant', 'Label', 'LiveIn', 'LiveOut',
    'Monitor', 'NOP', 'NullPtr', 'Parameter', 'Phi', 'Register',
    'SafePoint', 'SaveState', 'SaveStateDeoptimize', 'SaveStateOsr',
    'SpillFill', 'StaticAssertEQ', 'WrapObjectNative',
];

const optimization = [
    'Deoptimize', 'DeoptimizeCompare', 'DeoptimizeCompareImm',
    'DeoptimizeIf', 'FunctionImmediate', 'IsMustDeoptimize',
];

export const irdumpPlugin: LanguagePlugin = {
    id: 'irdump',
    monarchTokens: {
        controlFlow,
        arithmetic,
        bitwise,
        comparison,
        memory,
        object,
        cast,
        check,
        special,
        optimization,

        blockProperties: [
            'start', 'end', 'try', 'catch', 'loop', 'head', 'prehead', 'depth',
        ],

        types: [
            'void', 'bool', 'i32', 'i64', 'f32', 'f64', 'any', 'ref', 'ptr',
            'u32', 'u64', 'u16', 'u8', 'b',
        ],

        comparisonOps: ['EQ', 'NE', 'LT', 'GT', 'LE', 'GE'],

        tokenizer: {
            root: [
                // Section headers (multi-file view: # === filename ===)
                [/^# === .* ===$/, 'comment.irdump'],

                // Comments
                [/^#.*$/, 'comment.irdump'],

                // Method header
                [/^(Method:)/, 'keyword', '@methodHeader'],

                // Basic block header: BB 0, BB 1, etc.
                [/^(BB)(\s+)(\d+)/, ['keyword.block', 'white', 'number']],

                // Block properties line
                [/^(prop:)/, 'keyword.property.irdump'],

                // bb references in preds/succs: bb 0, bb 1
                [/bb/, 'number'],

                // hotness=N
                [/(hotness)(=)(\d+)/, ['keyword.property.irdump', 'delimiter', 'number']],

                // Block property keywords (same color as BB)
                [/\b(start|end|try|catch|loop|head|prehead|depth)\b/, 'keyword.block'],

                // Instruction line: "  14p.ref  Opcode" or "  1.  Opcode"
                // Split into number prefix (green) + type suffix (blue)
                [/^(\s+)(\d+p?\.)(void|i32|i64|f32|f64|ref|bool|b|ptr|any|u32|u64|u16|u8)(\s+)/,
                    ['white', 'number', 'type.irdump', { token: 'white', next: '@instruction' }]],
                // Instruction without type suffix: "  1.  Opcode"
                [/^(\s+)(\d+p?\.)(\s+)/,
                    ['white', 'number', { token: 'white', next: '@instruction' }]],

                // inlining_depth=N, file=N, caller=N
                [/inlining_depth=\d+/, 'keyword.property.irdump'],
                [/(?:file|caller)=\d+/, 'keyword.property.irdump'],

                [/\b(ACC)\b/, 'variable.physical'],

                // Dotted opcodes on continuation lines
                [/(?:CallStatic|CallVirtual|CallDirect|CallResolved|Call)\.Inlined\b/, 'keyword.flow'],
                [/Intrinsic\.[A-Z][a-zA-Z0-9_]+\b/, 'keyword.intrinsic'],

                // Opcodes on continuation lines (instruction spans multiple lines)
                [/[A-Z][a-zA-Z]+/, {
                    cases: {
                        '@controlFlow': 'keyword.flow',
                        '@arithmetic': 'keyword.arithmetic',
                        '@bitwise': 'keyword.bitwise',
                        '@comparison': 'keyword.comparison',
                        '@memory': 'keyword.memory',
                        '@object': 'keyword.object',
                        '@cast': 'keyword.cast',
                        '@check': 'keyword.check',
                        '@special': 'keyword.special',
                        '@optimization': 'keyword.optimization',
                        '@comparisonOps': 'keyword.operator',
                        '@default': 'identifier',
                    }
                }],
                [/\b(?:v?\d+p|v\d+)\/(?:v?\d+p|v\d+)\b/, 'variable.irdump'],
                [/\bv\d+[a-z]?\b/, 'variable.irdump'],
                [/\bvr\d+\b/, 'variable.physical'],
                [/\b(rax|rbx|rcx|rdx|rsi|rdi|rbp|rsp|r8|r9|r10|r11|r12|r13|r14|r15)\b/, 'variable.x64'],

                // Types as standalone
                [/\b(void|bool|i32|i64|f32|f64|any|ref|ptr|u32|u64|u16|u8|b)\b/, 'type.irdump'],

                // Hex constants
                [/\b0x[0-9a-fA-F]+\b/, 'number.hex.irdump'],

                // Decimal numbers
                [/\b\d+\b/, 'number'],

                // Strings
                [/'[^']*'/, 'string'],

                // Arrow — gray
                [/->/, 'variable.usage'],

                // Class/namespace references
                [/[\w.]+::/, 'type.identifier'],
            ],

            methodHeader: [
                // Types in method signature
                [/\b(void|bool|i32|i64|f32|f64|any|ref|ptr|u32|u64|u16|u8|b)\b/, 'type.irdump'],
                // Class::method pattern
                [/[\w.]+::/, 'type.identifier'],
                // Method name followed by (
                [/[\w]+\(/, 'entity.function.irdump'],
                // Hex address
                [/0x[0-9a-fA-F]+/, 'number.hex.irdump'],
                // End of line returns to root
                [/$/, '', '@pop'],
                // Everything else
                [/./, ''],
            ],

            instruction: [
                [/\b(ACC)\b/, 'variable.physical'],

                // Dotted opcodes: CallStatic.Inlined, Intrinsic.XxxYyy
                [/\b(?:CallStatic|CallVirtual|CallDirect|CallResolved|Call)\.Inlined\b/, 'keyword.flow'],
                [/\bIntrinsic\.[A-Z][a-zA-Z0-9_]+\b/, 'keyword.intrinsic'],

                // Opcodes by category
                [/[A-Z][a-zA-Z]+/, {
                    cases: {
                        '@controlFlow': 'keyword.flow',
                        '@arithmetic': 'keyword.arithmetic',
                        '@bitwise': 'keyword.bitwise',
                        '@comparison': 'keyword.comparison',
                        '@memory': 'keyword.memory',
                        '@object': 'keyword.object',
                        '@cast': 'keyword.cast',
                        '@check': 'keyword.check',
                        '@special': 'keyword.special',
                        '@optimization': 'keyword.optimization',
                        '@comparisonOps': 'keyword.operator',
                        '@default': 'identifier',
                    }
                }],

                // Builtin / Intrinsic as standalone
                [/\b(Builtin)\b/, 'keyword.call'],
                [/\b(Intrinsic)\b/, 'keyword.intrinsic'],
                [/\b(?:v?\d+p|v\d+)\/(?:v?\d+p|v\d+)\b/, 'variable.irdump'],
                [/\bv\d+[a-z]?\b/, 'variable.irdump'],
                [/\bvr\d+\b/, 'variable.physical'],
                [/\b(rax|rbx|rcx|rdx|rsi|rdi|rbp|rsp|r8|r9|r10|r11|r12|r13|r14|r15)\b/, 'variable.x64'],

                // arg N
                [/arg\s+\d+/, 'variable.irdump'],

                // bb references in succs/preds on instruction lines
                [/bb/, 'number'],

                // Types as operands
                [/\b(void|bool|i32|i64|f32|f64|any|ref|ptr|u32|u64|u16|u8|b)\b/, 'type.irdump'],

                // inlining_depth=N, file=N, caller=N
                [/inlining_depth=\d+/, 'keyword.property.irdump'],
                [/(?:file|caller)=\d+/, 'keyword.property.irdump'],

                // Hex constants
                [/\b0x[0-9a-fA-F]+\b/, 'number.hex.irdump'],

                // Decimal numbers
                [/\b\d+\b/, 'number'],

                // Strings
                [/'[^']*'/, 'string'],

                // Arrow + everything after it — gray, stops before bc:
                [/->.*?(?=\s*bc:)/, 'variable.usage'],
                // Arrow to end of line (no bc: on this line)
                [/->.*$/, 'variable.usage'],

                // Bytecode address label
                [/bc:/, 'keyword.property.irdump'],

                // Class references
                [/[\w.]+::/, 'type.identifier'],

                // Comparison operators
                [/\b(EQ|NE|LT|GT|LE|GE)\b/, 'keyword.operator'],

                // Brackets and delimiters
                [/[[\](){}]/, '@brackets'],
                [/[,]/, 'delimiter'],

                // End of line returns to root
                [/$/, '', '@pop'],
            ],
        },
    },
    configuration: {
        comments: {
            lineComment: '#',
        },
        brackets: [
            ['(', ')'],
            ['[', ']'],
            ['{', '}'],
        ],
        autoClosingPairs: [
            { open: '(', close: ')' },
            { open: '[', close: ']' },
            { open: '{', close: '}' },
            { open: '\'', close: '\'' },
        ],
        surroundingPairs: [
            { open: '(', close: ')' },
            { open: '[', close: ']' },
            { open: '{', close: '}' },
            { open: '\'', close: '\'' },
        ],
    },
};
