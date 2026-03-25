/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

export declare class TestStringCases {
    public hugeString: string;
    public chineseString: string;
    public emojiString: string;
    public specialCharsString: string;
    public mixedString: string;
    public emptyString: string;
    public whitespaceString: string;
}
export declare let testStringCasesInstance: TestStringCases;

export declare class TestStringMethods {
    public source: string;
    substring(start: number, end?: number): string;
    charAt(index: number): string;
    indexOf(searchString: string, position?: number): number;
    includes(searchString: string, position?: number): boolean;
    startsWith(searchString: string, position?: number): boolean;
    endsWith(searchString: string, position?: number): boolean;
    repeat(count: number): string;
    toUpperCase(): string;
    toLowerCase(): string;
    trim(): string;
}
export declare let testStringMethodsInstance: TestStringMethods;