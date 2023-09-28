{% for item in std_serialization_json_static_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_serialization_json_lib.j2' %}


{% for class_name in item.data_class_name_list %}
class {{.class_name}} {
    constructor({{.item.data_class_attributes_index[class_name]|join(',')}}) {
{%- for param_tuple in item.data_class_attributes_index[class_name] %}
	this.{{.param_tuple.split(":")|first}} = {{.param_tuple.split(":")|first}}
{%- endfor %}
    }
{%- for param_tuple in item.data_class_attributes_index[class_name] %}
    {{.param_tuple}}
{%- endfor %}
}
{% endfor %}
{% endfor %}
