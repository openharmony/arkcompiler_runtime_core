/*---
desc: >-
    Array access expression. The array reference expression must be checked to evaluate to null, and if
    so then the entire arrayAccessExpression equals null.
---*/

function getArrRef(): Int[]|null {
    return null;
}

function main(): void {
  assert getArrRef()?.[10] == null;
}
