{% for decl in groupedDeclarations %}
/*---
desc: Grouped declarations like \'let a = 1, b = 2\' cannot be exported
tags: [compile-only]
params: |
    {{decl}}
---*/

export {{decl}}

{%- endfor %}
