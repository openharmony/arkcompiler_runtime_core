/*---
desc: >-
  Scopes of two names can overlap (e.g., when statements are nested),
  in this case access to the outer name is not accessible because the
  name with the innermost declaration takes precedence.
---*/

let x = 1

function f1() {
    let x = 9
    assert x == 9
}

function main() {
    assert x == 1
    f1()
    assert x == 1
}
