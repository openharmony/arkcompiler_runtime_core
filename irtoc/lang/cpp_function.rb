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

require_relative 'output'

class CppFunction
  attr_reader :variants

  def initialize(name)
    @name = name
    @variants = []
  end

  def condition(cond)
    @current_variant.cond = cond
  end

  def code(&block)
    @current_variant.func = Function.new(@current_variant.name, params: @params, mode: [:IrInline], &block)
    @current_variant.func.compile
  end

  def params(**kwargs)
    @params = kwargs
  end

  def return_type(type)
    @return_type = type
  end

  def variant(name, &block)
    @current_variant = OpenStruct.new({condition: nil, name: name, func: nil})
    @variants << @current_variant
    self.instance_eval(&block)
  end

  def generate_ir(gen)
    @variants.each { |x| gen.generate_function(x.func) }

    params = "IntrinsicInst *inst, #{@params.keys.map { |x| "AnyBaseType #{x}"}.join(', ')}"
    Output.scoped_puts "inline #{@return_type} #{@name}(#{params}) {" do
      @variants.each do |variant|
        Output.scoped_puts "if (#{variant.cond}) {" do
          Output << "return #{variant.name}(inst);"
        end
      end
      Output << "return false;"
    end
  end
end