{% for c in cases %}
/*---
desc: The first element of the array is stored at the 0th index, 2nd element is stored on 1st index and so on
tags: [compile-only, negative]
---*/

let arr: int[] = [1, 2, 3]

function main(): void {
    arr[{{c.index}}] = 0;
}
{% endfor %}
