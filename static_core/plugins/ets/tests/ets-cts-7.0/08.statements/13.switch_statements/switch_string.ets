/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    Switch statement on String. Case-sensitive value equality (including new String(...) constructor
    as switch expression), string literal concatenation in case labels, function call as switch expression.
---*/

function switchMe(test: String): String {
    switch(test) {
        case 'Hello': return 'First'
        case 'World': return 'Second'
        default: return 'Sad'
    }
}

function switchCaseSensitive(s: String): String {
    switch(s) {
        case 'hello': return 'lowercase'
        case 'HELLO': return 'uppercase'
        case 'Hello': return 'capitalized'
        default: return 'unknown'
    }
}

// Verifies that new String(...) as switch expression uses value equality, not reference equality
function switchOnNewString(): String {
    switch(new String("World")) {
        case 'world': return 'wrong'
        case 'World': return 'correct'
        default: return 'other'
    }
}

// Verifies that string literal concatenation is a valid constant expression in case labels
function switchConcatInCase(s: String): String {
    switch(s) {
        case 'He' + 'llo': return 'matched'
        default: return 'unmatched'
    }
}

function getStr(): String {
    return 'target'
}

function getStrOther(): String {
    return 'nope'
}

// Verifies that a function call result can be used as switch expression
function switchOnMethodResult(s: String): String {
    switch(s) {
        case 'target': return 'matched'
        default: return 'other'
    }
}

function main() {
    arktest.assertEQ('First', switchMe('Hello'))
    arktest.assertEQ('Second', switchMe('World'))
    arktest.assertEQ('Sad', switchMe('Unknown'))

    arktest.assertEQ('capitalized', switchCaseSensitive('Hello'))
    arktest.assertEQ('lowercase', switchCaseSensitive('hello'))
    arktest.assertEQ('uppercase', switchCaseSensitive('HELLO'))
    arktest.assertEQ('unknown', switchCaseSensitive('hElLo'))  // default branch

    arktest.assertEQ('correct', switchOnNewString())

    arktest.assertEQ('matched', switchConcatInCase('Hello'))
    arktest.assertEQ('unmatched', switchConcatInCase('World'))

    arktest.assertEQ('matched', switchOnMethodResult(getStr()))
    arktest.assertEQ('other', switchOnMethodResult(getStrOther()))  // default branch
}
