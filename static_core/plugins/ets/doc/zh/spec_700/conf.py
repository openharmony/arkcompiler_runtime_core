#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys

sys.path.insert(0, os.path.abspath("../.."))

from sphinx.directives.code import CodeBlock
from docutils import nodes
import sphinx_common_conf
from docutils.parsers.rst import directives

project = u"{p} 规范".format(p=sphinx_common_conf.project)
author = sphinx_common_conf.author
copyright = sphinx_common_conf.copyright
version = "7.0.0"
release = "7.0.1"

rst_epilog = sphinx_common_conf.rst_epilog

language = "zh_CN"
today_fmt = sphinx_common_conf.default_today_fmt

extensions = ["sphinx_markdown_builder", "sphinx.ext.todo", "sphinxcontrib.plantuml"]
templates_path = []
source_suffix = [".rst"]
master_doc = "index"
exclude_patterns = []
pygments_style = None
html_theme = sphinx_common_conf.default_html_theme
htmlhelp_basename = "Documentationdoc"
latex_engine = "xelatex"

latex_elements = {
    "passoptionstopackages": r"\PassOptionsToPackage{bookmarksdepth=2}{hyperref}",
}

latex_documents = [
    (master_doc, "ArkTS-Sta_Specification_ch.tex", project, author, "manual"),
]


class CustomCodeBlock(CodeBlock):
    option_spec = CodeBlock.option_spec.copy()
    option_spec["language-to-compile"] = lambda arg: arg
    option_spec["donotcompile"] = directives.flag

    def run(self):
        language_value = self.options.get("language-to-compile")
        no_compile = "donotcompile" in self.options
        result = super().run()

        for node in result:
            if isinstance(node, nodes.literal_block):
                node["language-to-compile"] = language_value
                node["donotcompile"] = no_compile

        return result


def setup(app):
    app.add_directive("code-block", CustomCodeBlock, override=True)
