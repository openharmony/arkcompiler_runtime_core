/*---
desc: Enum cannot have two constants with equal names
tags: [negative, compile-only]
---*/

enum Colors {
    Red,
    {% for id in identifiers %} {{id}}, {%- endfor %}
    Red
}
