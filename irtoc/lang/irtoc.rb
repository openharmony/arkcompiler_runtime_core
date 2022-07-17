#!/usr/bin/env ruby

# Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

require_relative 'options'
# Parse options in advance, since it may be required during scripts loading
Options.parse

require_relative 'function'
require_relative 'cpp_function'
require_relative 'instructions_data'
require_relative 'output'
require_relative 'ir_generator'
require_relative 'isa'
require 'optparse'
require 'tempfile'
require 'yaml'

FILE_BEGIN = %{
// THIS FILE WAS GENERATED FOR #{Options.arch.upcase}

#include "irtoc/backend/compilation_unit.h"

#ifndef __clang_analyzer__

using namespace panda::compiler;

namespace panda::irtoc \{

}

FILE_END = %{
\} // namespace panda::irtoc

#endif  // __clang_analyzer__
}

class Irtoc

  attr_reader :functions, :cpp_functions

  def initialize
    # List of all compiled functions.
    @functions = []
    # Currently processed script file.
    @current_script = nil
    # Macros, that are defined for the current script. Since each macros is just a method in Function class, we need
    # to remove all these methods after script is done. Because further scripts should not see macros, defined in
    # previous one.
    @macros = []
    # Macros that also generate corresponding IR Builder methods
    @builder_macros = []
  end

  (0..31).each { |i| define_method("r#{i}") { i } }
  { :rax => 0,
    :rcx => 1,
    :rdx => 2,
    :rbx => 11,
    :rsp => 10,
    :rbp => 9,
    :rsi => 6,
    :rdi => 7}.each { |name, value| define_method(name) { value } }

  def function(name, **kwargs, &block)
    func = Function.new name, **kwargs, &block
    @functions << func unless func.mode.nil?
    func.compile
  end

  def cpp_function(name, &block)
    func = CppFunction.new(name)
    func.instance_eval(&block)
    (@cpp_functions ||= []) << func
  end

  def macro(name, &block)
    raise "Macro can't start with capital letter" if name.to_s.start_with? /[A-Z]/
    Function.define_method(name.to_sym, &block)
    @macros << name
  end

  def remove_macros
    @macros.each do |name|
      Function.remove_method name
    end
    @macros.clear
  end

  # Since Irtoc can be executed in parallel for the same files and from same working directory, we need to separate
  # generated files between these parallel runs
  def uniq_filename(base)
    num = (rand() * 100000).round.to_s(16)
    name = "#{base}.#{num}"
    begin
      yield name
    ensure
      File.delete(name) unless Options.keep_artifacts
    end
  end

  def include_relative(filename)
    uniq_filename("#{File.basename(filename)}.fixed") do |fixed_filename|
      filename = "#{File.dirname(@current_script)}/#{filename}"
      preprocess(filename, fixed_filename)
      self.instance_eval File.open(fixed_filename).read, filename
    end
  end

  def preprocess_line(line)
    line.gsub(/(\w+) *:= *([^>])/, 'let :\1, \2').gsub(/\} *Else/, "}; Else").gsub(/%(\w+)/, 'LiveIn(regmap[:\1])')
  end

  def read_plugin(filename)
    return "" if Options.plugins.empty?
    output = ""
    Options.plugins[filename].each do |full_filename|
      File.open(full_filename).readlines.each do |line|
        output += preprocess_line(line)
      end
    end
    output
  end

  def preprocess(filename, output_filename)
    File.open(output_filename, "w") do | file|
      File.open(filename).readlines.each do |line|
        line_matches = line.match(/include_plugin '(.*)'/)
        if line_matches
          filename = line_matches.captures[0]
          file.puts read_plugin(filename)
        else
          file.puts preprocess_line(line)
        end
      end
    end
  end

  def run(input_file)
    @current_script = input_file
    uniq_filename("#{File.basename(input_file)}.fixed") do |fixed_filename|
      preprocess(input_file, fixed_filename)
      begin
        data = File.open(fixed_filename).read
        self.instance_eval data, fixed_filename
      rescue SyntaxError => e
        puts "========== Begin #{fixed_filename} =========="
        puts data
        puts "========== End #{fixed_filename} =========="
        raise
      end
    end
  end
end

def main
  abort "YAML description file is not specified" unless Options.instructions_yaml
  abort "ISA YAML file is not specified" unless Options.isa
  abort "ISAPI file is not specified" unless Options.isapi

  InstructionsData.setup Options.instructions_yaml
  Function.setup
  IRInstruction.setup
  ISA.setup Options.isa, Options.isapi

  require Options.commonapi

  functions = []
  cpp_functions = []

  Options.input_files.each do |input_file|
    irtoc = Irtoc.new
    irtoc.run(input_file)
    functions += irtoc.functions
    cpp_functions += irtoc.cpp_functions if irtoc.cpp_functions
  end

  if Options.ir_api == 'ir-constructor'
    Output.setup Options.output_file
    Output.println(FILE_BEGIN)
    functions.reject(&:enable_builder).each(&:generate)
    Output.println(FILE_END)
  elsif Options.ir_api == 'ir-builder'
    builder_functions = functions.select(&:enable_builder)
    Output.setup Options.output_file
    Output.println('#include "optimizer/ir/graph.h"')
    Output.println('#include "optimizer/ir/basicblock.h"')
    Output.println('#include "optimizer/ir/inst.h"')
    Output.println('#include "asm_defines.h"')
    Output.println('namespace panda::compiler {')
    builder_functions.each(&:generate_builder)
    Output.println('} // namespace panda::compiler')
  elsif Options.ir_api == 'ir-inline'
    Output.setup Options.output_file
    Output << '#include "optimizer/ir/graph.h"'
    Output << '#include "optimizer/ir/basicblock.h"'
    Output << '#include "optimizer/ir/inst.h"'
    Output << "#include \"asm_defines.h\"\n"
    Output << "namespace panda::compiler {\n"
    cpp_functions.each { |func| func.generate_ir(GeneratorIrInline.new) }
    Output << '}  // namespace panda::compiler'
  else
    abort 'Should be unreachable: Unknown IR API'
  end

  validation = Hash[functions.select(&:validation).map { |f| [f.name, f.validation] } ]
  File.open("validation.yaml", "w") do |file|
    file.write(validation.to_yaml)
  end
end

main
