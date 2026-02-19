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

export const arkasmPlugin: LanguagePlugin = {
    id: 'arkasm',
    monarchTokens: {
        primitiveTypes: [
            'void', 'u1', 'u8', 'i8', 'u16', 'i16', 'u32', 'i32', 'i64', 'u64',
            'f32', 'f64', 'any',
        ],

        tokenizer: {
            root: [
                // Comments: # to end of line
                [/#.*$/, 'comment'],

                // Directives: .function, .record, .array, .catch, .catchall, .language
                [/\.(function|record|array|catch|catchall|language)\b/, 'keyword.block'],

                // Strings
                [/"/, 'string', '@string'],

                // Label definitions: word: alone on a line
                [/[\w$]+:(?=\s*$)/, 'tag'],

                // Function call with class prefix: std.core.Console.log:(
                [/([\w$<>\-]+(?:\.[\w$<>\-]+)*?\.)([\w$<>\-]+)(:\()/, ['type', 'function.arkasm', 'delimiter']],
                // Function call without prefix: .ctor:(
                [/(\.\w+)(:\()/, ['function.arkasm', 'delimiter']],

                // Hex numbers
                [/\b0x[0-9a-fA-F_]+\b/, 'number.hex'],

                // Decimal numbers (possibly negative)
                [/-?\d[\d_]*/, 'number'],

                // Registers: v0, v1, ...
                [/\bv\d+\b/, 'variable'],

                // Arguments: a0, a1, ...
                [/\ba\d+\b/, 'variable'],

                // Function declaration name
                [/([\w$<>\-]+(?:\.[\w$<>\-]+)*?\.)([\w$<>\-]+)(?=\s*\()/, ['type', 'function.arkasm']],

                // Qualified dotted names: std.core.Object, _ESTypeAnnotation.field
                [/[\w$][\w$<>\-]*(?:\.[\w$][\w$<>\-]*)+(?:\[\])*/, 'type'],

                // Attributes: <external>, <static>, <access.record>
                [/<[^>]+>/, 'delimiter'],

                // Primitive types vs instructions
                [/[a-z][\w.]*/, {
                    cases: {
                        '@primitiveTypes': 'type',
                        '@default': 'keyword',
                    }
                }],

                // Upper-case or underscore-prefixed identifiers (class/record names)
                [/[A-Z_][\w$]*/, 'type'],

                // Brackets and braces
                [/[{}()\[\]]/, 'delimiter'],

                // Delimiters
                [/[,:]/, 'delimiter'],

                // Operators
                [/=/, 'operator'],
            ],

            string: [
                [/[^\\"]+/, 'string'],
                [/\\./, 'string.escape'],
                [/"/, 'string', '@pop'],
            ],
        },
    },
    configuration: {
        comments: {
            lineComment: '#',
        },
        brackets: [
            ['{', '}'],
            ['[', ']'],
            ['(', ')'],
        ],
        autoClosingPairs: [
            { open: '{', close: '}' },
            { open: '[', close: ']' },
            { open: '(', close: ')' },
            { open: '"', close: '"' },
        ],
        surroundingPairs: [
            { open: '{', close: '}' },
            { open: '[', close: ']' },
            { open: '(', close: ')' },
            { open: '"', close: '"' },
        ],
    },
};
