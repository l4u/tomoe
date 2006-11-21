#!/usr/bin/env ruby

require 'rubygems'

require_gem 'rspec'
require 'spec'

test_dir = File.expand_path(File.dirname(__FILE__))
top_dir = File.expand_path(File.join(test_dir, ".."))
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby", ".libs"))
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby"))
$LOAD_PATH.unshift(File.join(test_dir))

class Spec::Runner::Formatter::ProgressBarFormatter
  def format_backtrace(backtrace)
    super.sub(/\A([^:]+:\d+)$/, '\\1:')
  end
end

Dir.glob(File.join(test_dir, "*_spec.rb")).each do |file|
  require File.basename(file, ".rb")
end
