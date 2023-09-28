/*---
desc: Char literals. Spec examples of char literals.
---*/

function main() {
    let c1 = c'a'
    let c2 = c'\n'
    let c3 = c'\x7F'
    let c4 = c'\u0000'

    let c1b = new Char(c1)
    let c2b = new Char(c2)
    let c3b = new Char(c3)
    let c4b = new Char(c4)

    assert c1b.unboxed() == c1
    assert c2b.unboxed() == c2
    assert c3b.unboxed() == c3
    assert c4b.unboxed() == c4
}
