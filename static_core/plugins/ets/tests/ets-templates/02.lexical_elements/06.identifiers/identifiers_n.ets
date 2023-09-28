{% for s in ids %}
/*---
desc: >-
    An identifier is a sequence of one or more valid Unicode characters.
    The Unicode grammar of identifiers is based on character properties specified by the Unicode Standard.
tags: [compile-only, negative]
---*/

let {{s}} = 0;

function main(): void {}
{%- endfor %}
