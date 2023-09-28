/*---
desc: >-
    Array access expression. The array access expression's result is a type T variable
    (i.e., a variable within the array selected by the value of that index expression).
---*/

let arr: (Int|null)[] = [new Int(1), new Int(2), null];

function getArrElem(i: int): Object|null {
    return arr[i];
}

function main(): void {
  assert getArrElem(0) instanceof Int;
  assert getArrElem(1) instanceof Object;
  assert getArrElem(2) == null;
}
