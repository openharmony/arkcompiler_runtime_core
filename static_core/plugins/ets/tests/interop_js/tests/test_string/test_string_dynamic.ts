/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let etsVm = globalThis.gtest.etsVm;
let testAll = etsVm.getFunction('Ltest_string_static/ETSGLOBAL;', 'testAll');

export class TestStringCases {
    hugeString: string = 'a'.repeat(10000000);
    chineseString: string = '你好世界！这是一个测试字符串。包含各种汉字：繁體字、한국어、日本語、العربية、Русский';
    emojiString: string = '😀😁😂🤣😃😄😅😆😉😊😋😎😍😘🥰😗😙😚☺️🙂🤗🤩🤔🤨😐😑😶🙄😏😣😥😮🤐😯😪😫😴😌😛😜😝🤤😒😓😔😕🙃🤑😲☹️🙁😖😞😟😤😢😭😦😧😨😩🤯😬😰😱🥵🥶😳🤪😵😡😠🤬😷🤯🤥😶‍🌫️🤫🔇🔈🔉🔊📢📣👁️‍🗨️💬🗨️🗯️💭💤👋🤚🖐️✋🖖👌🤏✌️🤞🤟🤘🤙👈👉👆🖕👇☝️👍👎✊👊🤛🤜👏🙌👐🤲🤝🙏✍️💅🤳💪🦾🦿🦵🦶👂🦻👃🧠🦷🦴👀👁👅👄👶🧒👦👧🧑👨👩🧓👱👴👵🧔👲👳👮👯👰👱👲👳👴👵👶👷👸👹👺👻👼👽👾👿💀☠️💩🤡👹👺👻👼👽👾👿';
    specialCharsString: string = '!@#$%^&*()_+-=[]{}|;:\'",.<>?/~`\\☺☻♥♦♣♠•◘○◙♂♀♪♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼␂␃␄␅␆␇␈⌂␍␎␏␐␑␒␓␔␕␖␗␘␙␚␛␜␝␞␟␠␡␢␣␤␥␦␧␨␩␪␫␬␭␮␯␰␱␲␳␴␵␶␷␸␹␺␻␼␽␾␿⑀⑁⑂⑃⑄⑅⑆⑇⑈⑉⑊⑋⑌⑍⑎⑏⑐⑑⑒⑓⑔⑕⑖⑗⑘⑙⑚⑛⑜⑝⑞⑟①②③④⑤⑥⑑⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳⑴⑵⑶⑷⑸⑹⑺⑻⑼⑽⑾⑿';
    mixedString: string = 'Hello世界🌍! 你好123#$%繁體한국어😊🎉测试🚀';
    emptyString: string = '';
    whitespaceString: string = '   \t\n\r  ';
}

export let testStringCasesInstance = new TestStringCases();

export class TestStringMethods {
    source: string = 'Hello World 你好世界 🌍';

    substring(start: number, end?: number): string {
        if (end !== undefined) {
            return this.source.substring(start, end);
        }
        return this.source.substring(start);
    }

    charAt(index: number): string {
        return this.source.charAt(index);
    }

    indexOf(searchString: string, position?: number): number {
        if (position !== undefined) {
            return this.source.indexOf(searchString, position);
        }
        return this.source.indexOf(searchString);
    }

    includes(searchString: string, position?: number): boolean {
        if (position !== undefined) {
            return this.source.includes(searchString, position);
        }
        return this.source.includes(searchString);
    }

    startsWith(searchString: string, position?: number): boolean {
        if (position !== undefined) {
            return this.source.startsWith(searchString, position);
        }
        return this.source.startsWith(searchString);
    }

    endsWith(searchString: string, position?: number): boolean {
        if (position !== undefined) {
            return this.source.endsWith(searchString, position);
        }
        return this.source.endsWith(searchString);
    }

    repeat(count: number): string {
        return this.source.repeat(count);
    }

    toUpperCase(): string {
        return this.source.toUpperCase();
    }

    toLowerCase(): string {
        return this.source.toLowerCase();
    }

    trim(): string {
        return this.source.trim();
    }
}

export let testStringMethodsInstance = new TestStringMethods();

function main(): void {
    testAll();
}

main();
