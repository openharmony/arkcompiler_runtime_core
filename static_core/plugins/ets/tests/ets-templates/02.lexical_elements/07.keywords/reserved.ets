{% for kwd in keywords %}
/*---
desc: The following identifiers are also treated as keywords and are reserved for the future use (or used in TS)
tags: [negative, compile-only]
params: '{{kwd}}'
---*/

function main(): void {
    let {{kwd}} = 0;
}
{% endfor %}
