/*---
desc: >-
    Array access expression. The array reference expression must be checked to evaluate to null, and if
    so then the entire arrayAccessExpression equals null.
---*/

let arr: Int[] = [new Int(11), new Int(22), new Int(33)];

function getArrRef(): Int[]|null {
    return arr;
}

function main(): void {
  assert getArrRef()?.[0] == 11;
}
