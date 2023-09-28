{% for case in cases %}
/*---
desc: Incorrect char literals
tags: [negative, compile-only]
---*/

function main(): int {
    let c: char;
    c = {{case}};
    return c as int;
}
{% endfor %}
