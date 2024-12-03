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

require 'optparse'
require 'ostruct'
require 'logger'
require 'fileutils'
require 'open3'

options = OpenStruct.new
OptionParser.new do |opts|
  opts.banner = 'Usage: checker.rb [options] TEST_FILE'

  opts.on('--run-prefix=PREFIX', 'Prefix that will be inserted before panda run command') do |v|
    options.run_prefix = v
  end
  opts.on('--source=FILE', 'Path to source file')
  opts.on('--test-file=FILE', 'Path to test file') do |v|
    options.test_file = v
  end
  opts.on('--panda=PANDA', 'Path to panda')
  opts.on('--paoc=PAOC', 'Path to paoc') do |v|
    options.paoc = v
  end
  opts.on('--frontend=FRONTEND', 'Path to frontend binary')
  opts.on('--panda-options=OPTIONS', 'Default options for panda run') do |v|
    options.panda_options = v
  end
  opts.on('--paoc-options=OPTIONS', 'Default options for paoc run') do |v|
    options.paoc_options = v
  end
  opts.on('--frontend-options=OPTIONS', 'Default options for frontend+bco run') do |v|
    options.frontend_options = v
  end
  opts.on('--method=METHOD', 'Method to optimize')
  opts.on('--command-token=STRING', 'String that is recognized as command start') do |v|
    options.command_token = v
  end
  opts.on('--release', 'Run in release mode. EVENT, INST and other will not be checked')
  opts.on('-v', '--verbose', 'Verbose logging')
  opts.on('--arch=ARCHITECTURE', 'Architecture of system where start panda')
  opts.on("--keep-data", "Do not remove generated data from disk") { |v| options.keep_data = true }
  opts.on("--with-llvm", "Tells checker that ARK was built with LLVM support") do |v|
    options.with_llvm = true
  end
end.parse!(into: options)

$LOG_LEVEL = options.verbose ? Logger::DEBUG : Logger::ERROR
$curr_cmd = nil

def log
  @log ||= Logger.new($stdout, level: $LOG_LEVEL)
end

def raise_error(msg)
  log.error "Test failed: #{$checker_name}"
  if !$current_method.nil?
    log.error "Method: \"#{$current_method}\""
  end
  if !$current_pass.nil?
    log.error $current_pass
  end
  log.error msg
  log.error "Command to reproduce: #{$curr_cmd}"
  raise msg
end

def match_str(match)
  match.is_a?(Regexp) ? "/#{match.source}/" : match
end

def contains?(str, match)
  return str =~ match if match.is_a? Regexp

  raise_error "Wrong type for search: #{match.class}" unless match.is_a? String
  str.include? match
end

# Provides methods to search lines in a given array
class SearchScope

  attr_reader :lines
  attr_reader :current_index

  def initialize(lines, name)
    @lines = lines
    @name = name
    @current_index = 0
  end

  def find_method_dump(match)
    @lines = @lines.drop(@current_index)
    @current_index = 0
    find(match)
    @lines = @lines.drop(@current_index - 1)
    @current_index = 0
    find(/}$/)
    @lines = @lines.slice(0, @current_index)
    @current_index = 0
  end

  def find_block(match)
    @lines = @lines.drop(@current_index)
    @current_index = 0
    find(match)
    @lines = @lines.drop(@current_index - 1)
    @current_index = 0
    find(/succs:/)
    @lines = @lines.slice(0, @current_index)
    @current_index = 0
  end

  def self.from_file(fname, name)
    SearchScope.new(File.readlines(fname), name)
  end

  def find(match)
    return if match.nil?

    @current_index = @lines.index { |line| contains?(line, match) }
    raise_error "#{@name} not found: #{match_str(match)}" if @current_index.nil?
    @current_index += 1
  end

  def find_next(match)
    return if match.nil?

    index = @lines.drop(@current_index).index { |line| contains?(line, match) }
    raise_error "#{@name} not found: #{match_str(match)}" if index.nil?
    @current_index += index + 1
  end

  def find_not(match)
    return if match.nil?

    @lines.each do |line|
      raise_error "#{@name} should not occur: #{match_str(match)}" if contains?(line, match)
    end
  end

  def find_next_not(match)
    return if match.nil?

    @lines.drop(@current_index).each do |line|
      raise_error "#{@name} should not occur: #{match_str(match)}" if contains?(line, match)
    end
  end

  def to_s
    "Scope '#{@name}', current=#{@current_index}\n#{@lines.join}"
  end
end

class Checker
  attr_reader :name

  def initialize(options, name)
    @name = name
    @lines = []
    @code = ""
    @cwd = "#{Dir.getwd}/#{name.gsub(/[ -:()]/, '_')}"
    @options = options
    @args = ''
    @ir_files = []
    @architecture = options.arch
    @aot_file = ''
    @llvm_paoc = false

    # Events scope for 'events.csv'
    @events_scope = nil
    # IR scope for IR dumps files 'ir_dump/*.ir'
    @ir_scope = nil

    # Disassembly file lines, that were read from 'disasm.txt'
    @disasm_lines = nil
    # Currently processing disasm method
    @disasm_method_scope = nil
    # Current search scope
    @disasm_scope = nil

    Dir.mkdir(@cwd) unless File.exists?(@cwd)
    clear_data
  end

  def set_llvm_paoc()
    @llvm_paoc = true
  end

  def append_line(line)
    @code << line + "\n"
  end

  def RUN(**args)
    expected_result = 0
    aborted_sig = 0
    entry = '_GLOBAL::main'
    env = ''
    @args = ''
    args.each do |name, value|
      if name == :force_jit and value
        @args << '--compiler-hotness-threshold=0 --no-async-jit=true --compiler-enable-jit=true '
      elsif name == :options
        @args << value
      elsif name == :entry
        entry = value
      elsif name == :result
        expected_result = value
      elsif name == :abort
        aborted_sig = value
      elsif name == :env
        env = value
      end
    end
    raise ":abort and :result cannot be set at the same time, :abort = #{aborted_sig}, :result = #{expected_result}" if aborted_sig != 0 && expected_result != 0

    clear_data
    aot_arg = @aot_file.empty? ? '' : "--aot-file #{@aot_file}"

    cmd = "#{@options.run_prefix} #{@options.panda} --compiler-queue-type=simple --compiler-ignore-failures=false #{@options.panda_options} \
                #{aot_arg} #{@args} --events-output=csv --compiler-dump --compiler-disasm-dump:single-file #{@options.test_file} #{entry}"
    $curr_cmd = "#{env} #{cmd}"
    log.debug "Panda command: #{$curr_cmd}"

    # See note on exec in RUN_PAOC
    output, status = Open3.capture2e("#{env} exec #{cmd}", chdir: @cwd.to_s)
    if aborted_sig != 0 && !status.signaled?
      puts output
      log.error "Expected ark to abort with signal #{aborted_sig}, but ark did not signal"
      raise_error "Test '#{@name}' failed"
    end
    if status.signaled?
      if status.termsig != aborted_sig
        puts output
        log.error "ark aborted with signal #{status.termsig}, but expected #{aborted_sig}"
        raise_error "Test '#{@name}' failed"
      end
    elsif status.exitstatus != expected_result
      puts output
      log.error "ark returns code #{status.exitstatus}, but expected #{expected_result}"
      raise_error "Test '#{@name}' failed"
    end
    log.debug output
    File.open("#{@cwd}/console.out", "w") { |file| file.write(output) }

    @events_scope = SearchScope.from_file("#{@cwd}/events.csv", 'Events')
  end

  def RUN_PAOC(**args)
    @aot_file = "#{Dir.getwd}/#{File.basename(@options.test_file, File.extname(@options.test_file))}.an"

    inputs = @options.test_file
    aot_output_option = '--paoc-output'
    output = @aot_file
    options = ''
    env = ''
    aborted_sig = 0
    result = 0

    args.each do |name, value|
      case name
      when :options
        options = value
      when :boot
        aot_output_option = '--paoc-boot-output'
      when :env
        env = value
      when :inputs
        inputs = value
      when :abort
        aborted_sig = value
      when :output
        output = value
      when :result
        result = value
      end
    end
    raise ":abort and :result cannot be set at the same time, :abort = #{aborted_sig}, :result = #{result}" if aborted_sig != 0 && result != 0

    paoc_args = "--paoc-panda-files #{inputs} --events-output=csv --compiler-dump #{options} #{aot_output_option} #{output}"

    clear_data

    cmd = "#{@options.run_prefix} #{@options.paoc} --compiler-ignore-failures=false --compiler-disasm-dump:single-file --compiler-dump #{@options.paoc_options} #{paoc_args}"
    $curr_cmd = "#{env} #{cmd}"
    log.debug "Paoc command: #{$curr_cmd}"

    # Using exec to pass signal info to the parent process.
    # Ruby invokes a process using /bin/sh if the curr_cmd has a metacharacter in it, for example '*', '?', '$'.
    # If an invoked process signals, then the status.signaled? check below returns different values depending on the shell.
    # For bash it is true, for dash it is false, because bash propagates a flag, whether the process has signalled or not.
    # When we use 'exec' we will propagate the signal too
    output, status = Open3.capture2e("#{env} exec #{cmd}", chdir: @cwd.to_s)
    if aborted_sig != 0 && !status.signaled?
      puts output
      log.error "Expected ark_aot to abort with signal #{aborted_sig}, but ark_aot did not signal"
      raise_error "Test '#{@name}' failed"
    end
    if status.signaled?
      if status.termsig != aborted_sig
        puts output
        log.error "ark_aot aborted with signal #{status.termsig}, but expected #{aborted_sig}"
        raise_error "Test '#{@name}' failed"
      end
    elsif status.exitstatus != result
      puts output
      log.error "ark_aot returns code #{status.exitstatus}, but expected #{result}"
      raise_error "Test '#{@name}' failed"
    end
    log.debug output
    File.open("#{@cwd}/console.out", "w") { |file| file.write(output) }

    @events_scope = SearchScope.from_file("#{@cwd}/events.csv", 'Events')
  end

  def RUN_AOT(**args)
    if @llvm_paoc
      RUN_LLVM(**args)
    else
      RUN_PAOC(**args)
    end
  end

  def RUN_BCO(**args)
    inputs = @options.test_file
    output = "#{@cwd}/#{File.basename(@options.test_file, '.*')}.abc"
    @args = ''

    args.each do |name, value|
      case name
      when :options
        @args << value
      when :inputs
        inputs = value
      when :output
        output = value
      when :method
        @args << "--bco-optimizer --method-regex=#{value}:.*"
      end
    end

    clear_data
    $curr_cmd = "#{@options.frontend} --opt-level=2 --dump-assembly --bco-compiler --compiler-dump \
            #{@options.frontend_options} #{@args} --output=#{output} #{@options.source}"
    log.debug "Frontend command: #{$curr_cmd}"

    # See note on exec in RUN_PAOC
    output, err_output, status = Open3.capture3("exec #{$curr_cmd}", chdir: @cwd.to_s)
    if status.signaled?
      if status.termsig != 0
        puts output
        log.error "#{@options.frontend} aborted with signal #{status.termsig}, but expected 0"
        raise_error "Test '#{@name}' failed"
      end
    elsif status.exitstatus != 0
      puts output
      log.error "#{@options.frontend} returns code #{status.exitstatus}, but expected 0"
      raise_error "Test '#{@name}' failed"
    elsif !err_output.empty?
      log.error "Bytecode optimizer failed, logs:"
      puts err_output
      raise_error "Test '#{@name}' failed"
    end
    File.open("#{@cwd}/console.out", "w") { |file| file.write(output) }
    Open3.capture2e("cat #{@cwd}/console.out")
    FileUtils.touch("#{@cwd}/events.csv")
  end

  def RUN_LLVM(**args)
    raise SkipException unless @options.with_llvm

    args[:options] << " --paoc-mode=llvm "
    RUN_PAOC(**args)
  end

  def EVENT(match)
    return if @options.release

    @events_scope.find(match)
  end

  def EVENT_NEXT(match)
    return if @options.release

    @events_scope.find_next(match)
  end

  def EVENT_COUNT(match)
    return 0 if @options.release

    @events_scope.lines.count { |event| contains?(event, match) }
  end

  def EVENT_NOT(match)
    return if @options.release

    @events_scope.find_not(match)
  end

  def EVENT_NEXT_NOT(match)
    return if @options.release

    @events_scope.find_next_not(match)
  end

  def EVENTS_COUNT(match, count)
    return if @options.release

    res = @events_scope.lines.count { |event| contains?(event, match) }
    raise_error "Events count missmatch for #{match}, expected: #{count}, real: #{res}" unless res == count
  end

  def TRUE(condition)
    return if @options.release

    raise_error "Not true condition: \"#{condition}\"" unless condition
  end

  class SkipException < StandardError
  end

  def SKIP_IF(condition)
    raise SkipException if condition
  end

  def IR_COUNT(match)
    return 0 if @options.release

    @ir_scope.lines.count { |inst| contains?(inst, match) && !contains?(inst, /^Method:/) }
  end

  def BLOCK_COUNT
    IR_COUNT('BB ')
  end

  def INST(match)
    return if @options.release

    @ir_scope.find(match)
  end

  def INST_NEXT(match)
    return if @options.release

    @ir_scope.find_next(match)
  end

  def INST_NOT(match)
    return if @options.release
    @ir_scope.find_not(match)
  end

  def INST_NEXT_NOT(match)
    return if @options.release

    @ir_scope.find_next_not(match)
  end

  def INST_COUNT(match, count)
    return if @options.release

    real_count = IR_COUNT(match)
    raise_error "IR_COUNT mismatch for #{match}: expected=#{count}, real=#{real_count}" unless real_count == count
  end

  def IN_BLOCK(match)
    return if @options.release

    @ir_scope.find_block(/prop: #{match}/)
  end

  def LLVM_METHOD(match)
    return if @options.release

    @ir_scope.find_method_dump(match)
  end

  def BC_METHOD(match)
    return if @options.release

    READ_FILE "console.out"
    @ir_scope.find_method_dump(/^\.function.*#{match.gsub('.', '-')}/)
  end

  module SearchState
    NONE = 0
    SEARCH_BODY = 1
    SEARCH_END = 2
  end

  def ASM_METHOD(match)
    ensure_disasm
    state = SearchState::NONE
    start_index = nil
    end_index = -1
    @disasm_lines.each_with_index do |line, index|
      case state
      when SearchState::NONE
        if line.start_with?('METHOD_INFO:') && contains?(@disasm_lines[index + 1].split(':', 2)[1].strip, match)
          state = SearchState::SEARCH_BODY
        end
      when SearchState::SEARCH_BODY
        if line.start_with?('DISASSEMBLY')
          start_index = index + 1
          state = SearchState::SEARCH_END
        end
      when SearchState::SEARCH_END
        if line.start_with?('METHOD_INFO:')
          end_index = index - 1
          break
        end
      end
    end
    raise "Method not found: #{match_str(match)}" if start_index.nil?

    @disasm_method_scope = SearchScope.new(@disasm_lines[start_index..end_index], "Method: #{match_str(match)}")
    @disasm_scope = @disasm_method_scope
  end

  def ASM_INST(match)
    ensure_disasm
    state = SearchState::NONE
    start_index = nil
    end_index = -1
    prefix = nil
    @disasm_method_scope.lines.each_with_index do |line, index|
      case state
      when SearchState::NONE
        if contains?(line, match)
          prefix = line.sub(/#.*/, '#').gsub("\n", '')
          start_index = index + 1
          state = SearchState::SEARCH_END
        end
      when SearchState::SEARCH_END
        if line.start_with?(prefix)
          end_index = index - 1
          break
        end
      end
    end
    raise "Can not find asm instruction: #{match}" if start_index.nil?

    @disasm_scope = SearchScope.new(@disasm_method_scope.lines[start_index..end_index], "Inst: #{match_str(match)}")
  end

  def ASM_RESET
    @disasm_scope = @disasm_method_scope
  end

  def ASM(**kwargs)
    ensure_disasm
    @disasm_scope.find(select_asm(kwargs))
  end

  def ASM_NEXT(**kwargs)
    ensure_disasm
    @disasm_scope.find_next(select_asm(kwargs))
  end

  def ASM_NOT(**kwargs)
    ensure_disasm
    @disasm_scope.find_not(select_asm(kwargs))
  end

  def ASM_NEXT_NOT(**kwargs)
    ensure_disasm
    @disasm_scope.find_next_not(select_asm(kwargs))
  end

  def select_asm(kwargs)
    kwargs[@options.arch.to_sym]
  end

  def ensure_disasm
    @disasm_lines ||= File.readlines("#{@cwd}/disasm.txt")
  end

  def METHOD(method)
    return if @options.release
    @ir_files = Dir["#{@cwd}/ir_dump/*#{method.gsub(/::|[<>]/, '_')}*.ir"]
    @ir_files.sort!
    raise_error "IR dumps not found for method: #{method.gsub(/::|[<>]/, '_')}" if @ir_files.empty?
    $current_method = method
    @current_file_index = 0
  end

  def PASS_AFTER(pass)
    return if @options.release

    $current_pass = "Pass after: #{pass}"
    @current_file_index = @ir_files.index { |x| File.basename(x).include? pass }
    raise_error "IR file not found for pass: #{pass}. Possible cause: you forgot to select METHOD first" unless @current_file_index
    @ir_scope = SearchScope.from_file(@ir_files[@current_file_index], 'IR')
  end

  def PASS_AFTER_NEXT(pass)
    return if @options.release

    $current_pass = "Pass after next: #{pass}"
    index = @ir_files[(@current_file_index + 1)..-1].index { |x| File.basename(x).include? pass }
    raise_error "IR file not found for pass: #{pass}. Possible cause: you forgot to select METHOD first" unless index
    @current_file_index += 1 + index
    @ir_scope = SearchScope.from_file(@ir_files[@current_file_index], 'IR')
  end

  def PASS_BEFORE(pass)
    return if @options.release

    $current_pass = "Pass before: #{pass}"
    @current_file_index  = @ir_files.index { |x| File.basename(x).include? pass }
    raise_error "IR file not found for pass: #{pass}. Possible cause: you forgot to select METHOD first" unless @current_file_index
    @ir_scope = SearchScope.from_file(@ir_files[@current_file_index - 1], 'IR')
  end

  def READ_FILE(filename)
    path = "#{@cwd}/#{filename}"
    raise_error "File `#{filename}` not found" unless File.file?(path)
    @ir_scope = SearchScope.from_file(path, 'Plain text')
  end

  def run
    log.info "Running \"#{@name}\""
    $checker_name = @name
    begin
      self.instance_eval @code
    rescue SkipException
      log.info "Skipped: \"#{@name}\""
    else
      log.info "Success: \"#{@name}\""
    end
    clear_data
  end

  def clear_data
   $current_method = nil
   $current_pass = nil
   if !@options.keep_data
      FileUtils.rm_rf("#{@cwd}/ir_dump")
      FileUtils.rm_rf("#{@cwd}/events.csv")
      FileUtils.rm_rf("#{@cwd}/disasm.txt")
      FileUtils.rm_rf("#{@cwd}/console.out")
   end
  end
end

def read_checks(options)
  checks = []
  check = nil
  check_llvm = nil
  command_token = /[ ]*#{options.command_token}(.*)/
  checker_start = /[ ]*#{options.command_token} CHECKER[ ]*(.*)/
  disabled_checker_start = /[ ]*#{options.command_token} DISABLED_CHECKER[ ]*(.*)/
  File.readlines(options.source).each do |line|
    if check
      unless line.start_with? command_token
        check = nil
        check_llvm = nil
        next
      end
      raise "No space between two checkers: '#{line.strip}'" if line.start_with? checker_start
      if line.include? "RUN_AOT"
        checks << check_llvm
      end
      check.append_line(command_token.match(line)[1]) unless check == :disabled_check
      check_llvm.append_line(command_token.match(line)[1]) unless check == :disabled_check
    else
      next unless line.start_with? command_token
      if line.start_with? checker_start
        name = command_token.match(line)[1]
        raise "Checker with name '#{name}'' already exists" if checks.any? { |x| x.name == name }

        check = Checker.new(options, name)
        check_llvm = Checker.new(options, "#{name} LLVMAOT")
        check_llvm.set_llvm_paoc()
        checks << check
      else
        raise "Line '#{line.strip}' does not belong to any checker" unless line.start_with? disabled_checker_start
        check = :disabled_check
        next
      end
    end
  end
  checks
end

def main(options)
  read_checks(options).each(&:run)
  0
end

if __FILE__ == $PROGRAM_NAME
  main(options)
end

# Somehow ruby resolves `Checker` name to another class in a Testing scope, so make this global
# variable to refer to it from unit tests. I believe there is more proper way to do it, but I
# didn't find it at first glance.
$CheckerForTest = Checker
