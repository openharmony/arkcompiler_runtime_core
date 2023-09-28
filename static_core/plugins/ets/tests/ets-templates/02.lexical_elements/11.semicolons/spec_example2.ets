/*---
desc: >-
    In most cases declarations and statements are terminated by a line separator (see §2.4).
    In some case a semicolon must be used to separate syntax productions written
    in one line or to avoid ambiguity.
tags: [compile-only]
---*/

function main(): void {
    let i = 1
    i-i++ // one expression
    i;-i++ // two expressions
}
