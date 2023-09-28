/*---
desc: The null type's single value is also null.
---*/



function main(): void {
    let a: Object|null = null;
    assert a instanceof null;
}
