#!/usr/bin/env ruby
# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

HEADER = %{
/**
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
*/

// Autogenerated file -- DO NOT EDIT!

}

def generate(input_file, output_file)
  data = File.read(input_file)
  defines = data.scan /"\^\^(\w+) [#\$]?([-+]?\d+)\^\^"/
  File.open(output_file, "w") do |file|
    file.puts HEADER
    defines.sort_by(&:first).each do |define|
      file.puts "// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)"
      file.puts "#define #{define[0]} #{define[1]}"
    end
  end
end

abort "Failed: input file required!" if ARGV.size < 2

generate ARGV[0], ARGV[1]
