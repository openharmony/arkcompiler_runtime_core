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

// Security-hardening regression tests for the native regexp layer:
//  - manual group-meta parse vs PCRE2 authoritative parse (3+ backslash runs)
//  - lookaround group mapping (manual index space vs PCRE2 capture space)
//  - tree-string (concatenated) inputs across all entry points: under GC the
//    flattened result must stay consistent (materialization / managed-scope run)
//  - mixed-encoding paths (LATIN1 pattern + UTF16 input and vice versa)
//  - replace/split loops that allocate on the managed heap between matches

function backslashRunsTest(): void {
    // 3 backslashes + '(' : PCRE2 parses as escaped-backslash + literal '(',
    // so the group must NOT be captured. A mis-parsed manual group meta would
    // report an extra (garbage) capture group.
    const re = new RegExp('\\\\\\(', '')
    const r = re.exec('a\\(b')
    arktest.assertEQ(r?.[0], '\\(')

    // 1 backslash + '(' : literal '(', no group
    arktest.assertEQ(new RegExp('\\(', '').exec('x(y')?.[0], '(')

    // 2 backslashes + group: escaped backslash, then a real capture group
    const rg = new RegExp('\\\\(a+)', '')
    const rr = rg.exec('z\\aaa')
    arktest.assertEQ(rr?.[0], '\\aaa')
    arktest.assertEQ(rr?.[1], 'aaa')

    // 4 backslashes + '(': two escaped backslashes, literal '('
    arktest.assertEQ(new RegExp('\\\\\\\\\\(', '').exec('q\\\\(w')?.[0], '\\\\(')
}

function lookaroundMappingTest(): void {
    // lookbehind is not countable in the manual meta, but the inner (a) is a
    // real PCRE2 capture: the mapping between manual group indices and PCRE2
    // captures must not be off by the lookaround.
    const re = new RegExp('(?<=1)(a)', 'g')
    const arr = Array.from('1a'.matchAll(re))
    arktest.assertEQ(arr.length, 1)
    arktest.assertEQ(arr[0][0], 'a')
    arktest.assertEQ(arr[0][1], 'a')

    // lookahead with optional capture
    const re2 = new RegExp('(?=(a)?)', 'g')
    const arr2 = Array.from('1a'.matchAll(re2))
    arktest.assertEQ(arr2[0][1], undefined)

    // multiple lookarounds interleaved with capture groups
    const re3 = new RegExp('(?<=x)(a)(?=y)(b)?', '')
    const r3 = re3.exec('xay')
    arktest.assertEQ(r3?.[0], 'a')
    arktest.assertEQ(r3?.[1], 'a')
    arktest.assertEQ(r3?.[2], undefined)
}

function treeStringInputTest(): void {
    // Build tree strings (concatenation does not flatten eagerly); every entry
    // point must read a stable snapshot even if internal flattening/GC occurs.
    let latin1: string = ''
    for (let i = 0; i < 200; i++) {
        latin1 += 'ab' + ((i % 10).toString()) + 'cd;'
    }
    const reG = new RegExp('ab[0-9]cd;', 'g')
    const m = latin1.match(reG)
    arktest.assertEQ(m?.length, 200)

    let utf16: string = ''
    for (let i = 0; i < 200; i++) {
        utf16 += 'あ' + ((i % 10).toString()) + 'い;'
    }
    const reU = new RegExp('あ[0-9]い;', 'gu')
    const mu = utf16.match(reU)
    arktest.assertEQ(mu?.length, 200)

    // tree-string pattern (built by concatenation) with a flat input
    let pat: string = 'a'
    for (let i = 0; i < 4; i++) {
        pat += '[bc]'
    }
    // pat == "a[bc][bc][bc][bc]" : matches 'a' followed by 4 chars of b/c
    arktest.assertTrue(new RegExp(pat, '').test('abcbc'))

    // exec/test/split/replace on the same tree input, results must agree
    const reOne = new RegExp('ab3cd;', '')
    arktest.assertTrue(reOne.test(latin1))
    const ex = reOne.exec(latin1)
    arktest.assertEQ(ex?.[0], 'ab3cd;')
    const parts = latin1.split(reG)
    arktest.assertEQ(parts.length, 201)
    const rep = latin1.replace(reG, 'X')
    arktest.assertEQ(rep.match(new RegExp('X', 'g'))?.length, 200)
}

function mixedEncodingTest(): void {
    // LATIN1 pattern + UTF16 input (LATIN1_TO_UTF16 path)
    const reL = new RegExp('b[0-9]', 'g')
    const u16 = 'あb1いb2う'
    arktest.assertEQ(u16.match(reL)?.length, 2)

    // UTF16 pattern + LATIN1 input (pattern flattening + input fast path)
    const reU = new RegExp('あ[0-9]', 'g')
    const l1 = 'xあ1yあ2z'
    arktest.assertEQ(l1.match(reU)?.length, 2)

    // unicode flag forces UTF16 execution on LATIN1 data
    const reUN = new RegExp('\\u0041', 'u')
    arktest.assertTrue(reUN.test('A'.repeat(64)))

    // literal replace across encodings
    const out1 = u16.replace(reL, 'Y')
    arktest.assertEQ(out1, 'あYいYう')
    const out2 = l1.replace(reU, 'Y')
    arktest.assertEQ(out2, 'xYyYz')
}

function allocatingLoopTest(): void {
    // replace/split loops allocate result objects between matches; combined
    // with GC pressure (many intermediate strings) the input snapshot must
    // remain stable for every subsequent match. Uses literal replacement (the
    // native substitute path covered by the hardening) for the large-input
    // loop; $-substitution is verified on small flat inputs below to stay
    // independent of tree-string/GC-specific behavior.
    const chunk = 'k1v1;k2v2;k3v3;'
    let big: string = ''
    for (let i = 0; i < 300; i++) {
        big += chunk
    }
    const out = big.replace(new RegExp('k[0-9]v[0-9];', 'g'), 'kXvY;')
    arktest.assertEQ(out.length, big.length)
    arktest.assertEQ(out.substring(0, 5), 'kXvY;')

    // $-substitution expansion on small flat inputs
    arktest.assertEQ('k1v1;'.replace(new RegExp('k([0-9])v([0-9]);', 'g'), 'k$1v$2;'), 'k1v1;')
    arktest.assertEQ('k3v2;'.replace(new RegExp('k([0-9])v([0-9]);', 'g'), 'k$1v$2;'), 'k3v2;')
    arktest.assertEQ('k9v8;'.replace(new RegExp('k([0-9])v([0-9]);', 'g'), 'v$2k$1;'), 'v8k9;')

    // split into many substrings, verify content integrity of all pieces
    const pieces = big.split(new RegExp(';', 'g'))
    arktest.assertEQ(pieces.length, 300 * 3 + 1)
    arktest.assertEQ(pieces[0], 'k1v1')
    arktest.assertEQ(pieces[1], 'k2v2')

    // matchAll with named groups under allocation pressure
    const reN = new RegExp('k(?<idx>[0-9])v(?<val>[0-9]);', 'g')
    let seen = 0
    for (const m of big.matchAll(reN)) {
        // CC-OFFNXT(G.OBJ.04)
        if (m.groups?.['idx'] === '1' && m.groups?.['val'] === '1') {
            seen++
        }
    }
    arktest.assertEQ(seen, 300)
}

function namedGroupsBeyond255Test(): void {
    // PCRE2's 8-bit name table stores group numbers as 2 big-endian bytes. Group numbers
    // >= 256 (i.e. more than 255 capture groups in one pattern) must not lose their high
    // byte: a truncated number aliased group 256 to the whole match (number 0) and group
    // 257 to group 1, silently corrupting named-group results on exec/matchAll/replace.
    let pattern: string = ''
    for (let i = 0; i < 300; i++) {
        pattern += '(?<g' + i.toString() + '>.)'
    }
    // Distinct single-byte chars (28..227 keep the input Latin-1); the values below are
    // chosen so that every asserted group differs from its truncated (buggy) counterpart.
    let input: string = ''
    for (let i = 0; i < 300; i++) {
        input += String.fromCharCode(28 + (i % 200))
    }

    // LATIN1 path (RegExp8 ExtractGroups)
    const r = new RegExp(pattern, '').exec(input)
    arktest.assertEQ(r?.[0], input)
    // CC-OFFNXT(G.OBJ.04) bracket notation required for Record/JsonElement/indexable types
    arktest.assertEQ(r?.groups?.['g0'], String.fromCharCode(28))
    // CC-OFFNXT(G.OBJ.04) bracket notation required for Record/JsonElement/indexable types
    arktest.assertEQ(r?.groups?.['g254'], String.fromCharCode(28 + 54))
    // g255 is capture group 256 (0x0100): the high byte is significant
    // CC-OFFNXT(G.OBJ.04) bracket notation required for Record/JsonElement/indexable types
    arktest.assertEQ(r?.groups?.['g255'], String.fromCharCode(28 + 55))
    // g256 is capture group 257: must NOT alias to group 1 (g0's value)
    // CC-OFFNXT(G.OBJ.04) bracket notation required for Record/JsonElement/indexable types
    arktest.assertEQ(r?.groups?.['g256'], String.fromCharCode(28 + 56))
    // CC-OFFNXT(G.OBJ.04) bracket notation required for Record/JsonElement/indexable types
    arktest.assertEQ(r?.groups?.['g299'], String.fromCharCode(28 + 99))

    // UTF16 execution path (RegExp16 ExtractGroups, LATIN1_TO_UTF16)
    const ru = new RegExp(pattern, 'u').exec(input)
    // CC-OFFNXT(G.OBJ.04) bracket notation required for Record/JsonElement/indexable types
    arktest.assertEQ(ru?.groups?.['g255'], String.fromCharCode(28 + 55))
    // CC-OFFNXT(G.OBJ.04) bracket notation required for Record/JsonElement/indexable types
    arktest.assertEQ(ru?.groups?.['g299'], String.fromCharCode(28 + 99))

    // matchAll path (ParseNameTable)
    const matches = Array.from(input.matchAll(new RegExp(pattern, 'g')))
    arktest.assertEQ(matches.length, 1)
    // CC-OFFNXT(G.OBJ.04) bracket notation required for Record/JsonElement/indexable types
    arktest.assertEQ(matches[0].groups?.['g255'], String.fromCharCode(28 + 55))
    // CC-OFFNXT(G.OBJ.04) bracket notation required for Record/JsonElement/indexable types
    arktest.assertEQ(matches[0].groups?.['g299'], String.fromCharCode(28 + 99))

    // $<name> substitution reads the same named-group table
    arktest.assertEQ(input.replace(new RegExp(pattern, 'g'), '$<g299>'), String.fromCharCode(28 + 99))
}

function lastIndexResetTest(): void {
    // after a failed global match, lastIndex must be reset to 0
    const re = new RegExp('z', 'g')
    re.lastIndex = 10
    arktest.assertFalse(re.test('aaaa'))
    arktest.assertEQ(re.lastIndex, 0)

    // sticky regex failing at lastIndex resets too
    const rs = new RegExp('a', 'y')
    rs.lastIndex = 3
    arktest.assertFalse(rs.test('aaa'))
    arktest.assertEQ(rs.lastIndex, 0)
}

function main(): int {
    const suite = new arktest.ArkTestsuite('RegExp.NativeHardening')

    suite.addTest('backslashRunsTest', backslashRunsTest)
    suite.addTest('lookaroundMappingTest', lookaroundMappingTest)
    suite.addTest('treeStringInputTest', treeStringInputTest)
    suite.addTest('mixedEncodingTest', mixedEncodingTest)
    suite.addTest('allocatingLoopTest', allocatingLoopTest)
    suite.addTest('lastIndexResetTest', lastIndexResetTest)

    return suite.run()
}
