/*---
desc: >-
    An identifier is a sequence of one or more valid Unicode characters.
    The Unicode grammar of identifiers is based on character properties specified by the Unicode Standard.
tags: [compile-only]
---*/

{% for s in id_start %}
{% for c in id_continue %}
let {{s}}{{c}} = 0;
{%- endfor %}
{%- endfor %}

function main(): void {}
