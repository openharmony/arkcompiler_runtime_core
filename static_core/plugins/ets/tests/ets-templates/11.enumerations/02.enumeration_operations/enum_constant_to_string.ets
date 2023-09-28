/*---
desc: Enum constant should have a toString method that returns its value as a string
---*/

enum GeneratedEnum {
    {% for length in range(1, 10) -%}
    {% for _ in range(length)%}A{% endfor %}{%if length is lt(9)%},{% endif %}
    {% endfor %}
}


function main(): void {
    {% for i in range(1, 10) %}
    assert(GeneratedEnum.{% for _ in range(i)%}A{% endfor %}.toString() == "{{i-1}}");
    {%- endfor %}
}
