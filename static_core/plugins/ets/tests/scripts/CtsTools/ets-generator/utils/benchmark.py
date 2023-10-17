from utils.fsutils import write_file
from utils.exceptions import *
from utils.params import generate_params
from utils.template import Template
from utils.log import *


TEMPLATE_EXTENSION = ".ets"
OUT_EXTENSION = ".ets"


class Benchmark():
    def __init__(self, input, output, test_full_name):
        self.name = input.name

        self.input = input
        self.output = output.parent  
        self.full_name = self.__get_full_name(test_full_name) 

    def generate(self):
        info(f"Generating test: {self.name}")
        name_without_ext, _ = self.name.split(".")
        params = generate_params(self.input, name_without_ext)
        self.template = Template(self.input, params)
        
        info(f"Starting generate test template: {self.name}")
        tests = self.__get_all_variants()

        assert len(tests) > 0, "Internal error: there should be tests"

        if len(tests) == 1:
            test_text = self.template.generate_template(tests[0], self.full_name)
            write_file(path=self.output.joinpath(f"{name_without_ext}{OUT_EXTENSION}"), text=test_text)
        else:
            for i, test in enumerate(tests):
                name = f"{name_without_ext}_{i}"
                full_name = f"{self.full_name}_{i}"

                test_text = self.template.generate_template(test, full_name)
                output_filepath = self.output.joinpath(f"{name}{OUT_EXTENSION}")
                write_file(path=output_filepath, text=test_text) 
        info(f"Finish generating test template for: {self.name}")  

    def __get_full_name(self, name):
        return name[:-len(TEMPLATE_EXTENSION)]

    def __get_all_variants(self):
        return self.template.render_template()   
