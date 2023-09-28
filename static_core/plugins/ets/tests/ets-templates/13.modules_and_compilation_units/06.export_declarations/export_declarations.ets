{% for decl in topLevelDeclarations %}
/*---
desc: \'export\' keyword can be added to any top level declaration
tags: [compile-only]
params: |
    {{decl}}
---*/

export {{decl}}

{%- endfor %}
