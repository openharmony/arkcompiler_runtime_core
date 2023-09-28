{% for kwd in keywords %}
/*---
desc: The following words have special meaning in certain contexts (soft keywords) but are valid identifiers in other places.
tags: [compile-only]
params: '{{kwd}}'
---*/

function main(): void {
    let {{kwd}} = 0;
}
{% endfor %}
