{% for kwd in keywords %}
/*---
desc: The following keywords are reserved in any context (hard keywords) and cannot be used as identifiers.
tags: [negative, compile-only]
params: '{{kwd}}'
---*/

function main(): void {
    let {{kwd}} = 0;
}
{% endfor %}
