{% for se in shift_errors %}
/*---
desc: Shift of int type, large shift value
tags: [negative]
---*/

function main(): void {
    let v: {{se.type}} = 1;
    let k = v {{se.shift}} {{se.value}};
}

{% endfor %}