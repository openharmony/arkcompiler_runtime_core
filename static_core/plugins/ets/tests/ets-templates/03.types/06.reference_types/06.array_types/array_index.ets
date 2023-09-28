/*---
desc: The first element of the array is stored at the 0th index, 2nd element is stored on 1st index and so on
---*/

function main(): void {
    let x: int[] = [1, 2, 3];
    assert(x[0 as byte] == 1);
    assert(x[1 as short] == 2);
    assert(x[0 as int] == 1);
    assert(x[1 as char] == 2);
}
