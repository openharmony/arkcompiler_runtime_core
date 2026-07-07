/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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


function isPalindrom(text: String): boolean {
    let halve: int = text.length.toInt() / 2;
    for (let i: int = 0; i < halve; i++) {
        if (text.charAt(i) != text.charAt(text.length.toInt() - i - 1)) {
            return false;
        }
    }
    return true;
}

function randomString(len: int): String {
    const t = 'qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM';
    const l = t.length;
    let sb = new StringBuilder();
    for (let i = 0; i < len; i++) {
        sb.append(t.charAt((random() * l).toInt()));
    }
    return sb.toString();
}

function test(result: boolean, name: String): int {
    if (result) {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.println('PASSED: ' + name);
        return 0;
    }
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.println('FAILED: ' + name);
    return 1;
}

function main(): int {
    let failures: int = 0;
    failures += test(isPalindrom('ABBA'), 'ABBA is a palindrom');
    failures += test(isPalindrom('KABAK'), 'KABAK is a palindrom');
    failures += test(!isPalindrom('ABbA'), 'ABbA is not a palindrom');
    for (let i = 0; i < 5; i++) {
        let word = 'a' + randomString(60) + 'z';
        failures += test(!isPalindrom(word), word + ' is not a palindrom');
    }
    return failures;
}
