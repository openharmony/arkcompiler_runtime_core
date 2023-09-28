/*---
desc: The type of a declared entity is Object | null if the initializer expression is a null literal (null).
---*/

let x = null;  // inferred Object|null

function main(): void {
    x = new Error();
    assert x instanceof Error;

    x = null;
    assert x == null;

    x = new Object();
    assert x instanceof Object;

    x = new string[1];
    (x as string[])[0] = "abc";
    assert (x as string[])[0] == "abc";
}
