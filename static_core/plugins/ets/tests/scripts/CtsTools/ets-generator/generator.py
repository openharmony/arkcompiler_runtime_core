#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

import argparse
import os
import shutil
from pathlib import Path
from utils.exceptions import *
from utils.benchmark import Benchmark
from utils.log import *
from utils.benchmark import TEMPLATE_EXTENSION

class BenchGenerator():
    __output_path = None
    __test_path = None

    def clean_output_dir(self):
        info("Removing old directory")
        shutil.rmtree(self.__output_path)

    def process_test(self, input):
        test_full_name = os.path.relpath(input, self.__test_path)
        output = self.__output_path.joinpath(test_full_name)
        bench = Benchmark(input, output, test_full_name)
        bench.generate()

    def process_input(self, input, seen):
        input = input.resolve()
        if not input.exists():
            return
        seen.add(input)
        if input.is_dir():
            for i in sorted(input.iterdir()):
                self.process_input(i, seen)
            return
        elif input.suffix == TEMPLATE_EXTENSION:
            self.process_test(input)
            return

    def main(self, cmd_args):
        info("Starting generate test")
        self.__test_path = Path(cmd_args.test_dir).resolve()
        self.__output_path = Path(cmd_args.output_dir).resolve()

        if(self.__output_path.exists()):
            self.clean_output_dir()

        if not self.__test_path.is_dir():
            exception(f"{str(self.__test_path.absolute())} must be a directory")
            exit(1)
            
        try:
            self.process_input(self.__test_path, set())
        except InvalidFileFormatException as e:
            exception(e.message)
            exit(1)
        except InvalidFileStructureException as e:
            exception(e.message)
            exit(1)
        except UnknownTemplateException as e:
            exception(f"{e.filepath}: exception while processing template:")
            exception("\t" + repr(e.exception))
            exit(1)

        info("Generation finished!")


class ArgumentParser(argparse.ArgumentParser):
    @staticmethod
    def __epilog():
        return (
            "Generator for STS CTS tests"
        )

    def __init__(self):
        super().__init__(
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog=self.__epilog()
        )
        self.add_argument("-t", "--test-dir", type=str, 
                        help="Path to a directory that contains test templates and parameters", 
                        required=True)
        self.add_argument("-o", "--output-dir", type=str, 
                        help="Path to output directory. Output directory and all"  +
                        " subdirectories are created automatically", 
                        required=True)

if __name__ == "__main__":
    parser = ArgumentParser()
    cmd_args = parser.parse_args()
    bg = BenchGenerator()
    bg.main(cmd_args)
