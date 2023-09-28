{% for c in cases %}
/*---
desc: >-
    If two functions declared in the same declaration space
    have the same name but different signatures that are not
    override-equivalent, then the function name is said to be overloaded.
params: same-signature functions
tags: [compile-only, negative]
---*/

{{c.defs}}

function main(): int {
    {{c.use}}
    return 0;
}
{% endfor %}
