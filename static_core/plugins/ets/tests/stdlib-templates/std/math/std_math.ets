{% for item in std_math_trigonometry %}

/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_core_math_lib.j2' %}

{% endfor %}

{% for item in std_math_h_trigonometry %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_core_math_lib.j2' %}
{% endfor %}

{% for item in std_math_log %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_core_math_lib.j2' %}
{% endfor %}

{% for item in std_math_exp %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_core_math_lib.j2' %}
{% endfor %}

{% for item in std_math_bits %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_core_math_lib.j2' %}
{% endfor %}

{% for item in std_math_arithmetic %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature.values()|join("_")}}}
---*/

{% include 'utils/test_core_math_lib.j2' %}
{% endfor %}
