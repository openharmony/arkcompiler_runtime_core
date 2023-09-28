import re

# Meta
META_START_STRING = "/*---"
META_START_PATTERN = re.compile(r"\/\*---")
META_END_STRING = "---*/"
META_END_PATTERN = re.compile(r"---\*\/")
META_START_COMMENT = "/*"
META_END_COMMENT = "*/"
META_COPYRIGHT = "Copyright (c)"

# Extensions
YAML_EXTENSIONS = [".yaml", ".yml"]
TEMPLATE_EXTENSION = ".ets"
OUT_EXTENSION = ".ets"
JAR_EXTENSION = ".jar"

# Prefixes
LIST_PREFIX = "list."
NEGATIVE_PREFIX = "n."
NEGATIVE_EXECUTION_PREFIX = "ne."
SKIP_PREFIX = "tbd."

# Jinja
VARIABLE_START_STRING = "{{."

# Spec
SPEC_SECTION_TITLE_FIELD_NAME = "name"
SPEC_SUBSECTIONS_FIELD_NAME = "children"
