/*---
desc: >-
    The scope of a name declared immediately within the body of a function (method)
    declaration is the body of that function declaration (method or function scope)
    from the place of declaration and up to the end of the body.
---*/

let x = 9

function f1(p: int) {
    let x = p + 1
    if (true) {
        if (true) {
            let x = 20
            let p = 30
            assert x == 20 && p == 30
        }
        assert x == 10 && p == 9
    }
}

function main() {
    f1(x)
    assert x == 9
}
