{% for i in range(100, 1000, 100) %}
/*---
desc: Enums must support any number of constants
tags: [compile-only]
params: number of constants = {{i}}
---*/

enum GeneratedEnum {
    {% for length in range(1, i) -%}
    {% for _ in range(length)%}A{% endfor %}{%if length is lt(i - 1)%},{% endif %}
    {% endfor %}
}

{%- endfor %}
