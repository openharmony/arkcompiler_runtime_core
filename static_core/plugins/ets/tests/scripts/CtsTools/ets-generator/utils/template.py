import os
import re
import yaml
from utils.fsutils import read_file
from jinja2 import Environment, select_autoescape, TemplateSyntaxError
from utils.exceptions import *
from utils.log import *


META_START_STRING = "/*---"
META_START_PATTERN = re.compile("\/\*---")
META_END_STRING = "---*/"
META_END_PATTERN = re.compile("---\*\/")
META_COPYRIGHT = "Copyright (c)"
META_START_COMMENT = "/*"
META_END_COMMENT = "*/"


SCR_PATH = os.path.realpath(os.path.dirname(__file__))
ROOT_PATH = os.path.join(SCR_PATH, "..")


BENCH_TEMPLATE = read_file(os.path.join(ROOT_PATH, "test_template.tpl"))
COPYRIGHT = read_file(os.path.join(ROOT_PATH, "copyright.txt"))


class Template():
    env = Environment(
        autoescape=select_autoescape()
    )

    def __init__(self, filepath, params):
        self.filepath = filepath
        self.text = read_file(filepath)
        self.is_copyright = self.__check_copyright()
        self.params = params

    def render_template(self):
        self.__add_copyright()
        
        try:   
            template = self.env.from_string(self.text)
            self.render = template.render(**self.params).strip()
        except TemplateSyntaxError as e:
            raise InvalidFileFormatException(message=f"Template Syntax Error: {e.message}", filepath=self.filepath)
        except Exception as e:
            raise UnknownTemplateException(filepath=self.filepath, exception=e)

        tests = self.__split_render_result()

        return tests

    def __split_render_result(self):   
        result = []

        if self.is_copyright: 
            result = [META_START_STRING + e for e in self.render.split(META_START_STRING) if e]
        else:
            result = [META_START_COMMENT + e for e in self.render.split(META_START_COMMENT) if e]

        return result

    def __find_meta(self, text):
        start_indice = text.find(META_START_STRING)
        end_indice = text.find(META_END_STRING)

        if start_indice > end_indice or start_indice == -1 or end_indice == -1:
            raise InvalidMetaException("Invalid meta or meta does not exist")

        return [start_indice, end_indice]

    def __find_copyright(self, text):
        start_indice = text.find(META_START_COMMENT)
        end_indice = text.find(META_END_COMMENT)

        if start_indice > end_indice or start_indice == -1 or end_indice == -1:
            raise InvalidMetaException("Invalid meta or meta does not exist")

        return [start_indice, end_indice]

    def __remove_substring(self, text, start, end):       
        return text[:start] + text[end:]

    def __parse_meta(self, text):
        test_text = self.__decode_text(text)
        meta_bounds = self.__find_meta(test_text)
        result = dict()

        start = meta_bounds[0]
        end = meta_bounds[1] + len(META_END_STRING)

        content = test_text[meta_bounds[0] + len(META_START_STRING):meta_bounds[1]]

        result['config'] = self.__parse_text(content)
        result['code'] = self.__remove_substring(test_text, start, end)

        return result

    def __decode_text(self, text):
        codes = (
            ("'", '&#39;'),
            ('"', '&#34;'),
            ('>', '&gt;'),
            ('<', '&lt;'),
            ('&', '&amp;')
        )

        for code in codes:
            text = text.replace(code[1], code[0])

        return text

    def __check_copyright(self):
        if not META_COPYRIGHT in self.text:
            return True
        else:
            return False

    def __add_copyright(self):
        if not META_COPYRIGHT in self.text:
            self.copyright = COPYRIGHT
        else:
            meta_bounds = self.__find_copyright(self.text)
            self.copyright = self.text[meta_bounds[0] + len(META_START_COMMENT):meta_bounds[1]]
            start = meta_bounds[0]
            end = meta_bounds[1] + len(META_END_COMMENT)
            self.text = self.__remove_substring(self.text, start, end).strip()
            
    def __parse_text(self, text):
        try:
            result = yaml.load(text, Loader=yaml.FullLoader)
        except Exception as e:
            raise InvalidFileFormatException(message=f"Could not load YAML in test params: {str(e)}", filepath=self.filepath)

        return result

    def generate_template(self, test, fullname):
        meta = self.__parse_meta(test)
        config = meta['config']
        code = meta['code']
        params = []
        
        config["name"] = fullname

        yaml_content = yaml.dump(config)

        bench_code = BENCH_TEMPLATE.format(
                    copyright = self.copyright.strip(),
                    description=yaml_content.strip(),
                    code=code.strip())

        return bench_code

