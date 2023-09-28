/*---
desc: Enum constant should have a ordinal() method that returns its index starting from 0
---*/

enum GeneratedEnum {
    {% for length in range(1, 10) -%}
    {% for _ in range(length)%}A{% endfor %}{%if length is lt(9)%},{% endif %}
    {% endfor %}
}


function main(): void {
    {% for i in range(0, 9) %}
    assert(GeneratedEnum.{% for _ in range(i + 1)%}A{% endfor %}.getValue() == {{i}});
    {%- endfor %}
}
