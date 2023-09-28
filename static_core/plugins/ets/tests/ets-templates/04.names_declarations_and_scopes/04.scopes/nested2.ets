/*---
desc: >-
  Scopes of two names can overlap (e.g., when statements are nested),
  in this case access to the outer name is not accessible because the
  name with the innermost declaration takes precedence.
---*/

let x = 1

function f1(): int {
    return x;
}

function main() {
    let x = 3
    if (true) {
        let x: int
        assert f1() == 1
        assert x == 0
    }
    assert x == 3
}
