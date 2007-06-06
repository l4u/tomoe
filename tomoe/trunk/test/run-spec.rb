#!/usr/bin/env ruby

require 'rubygems'

require 'spec'

test_dir = File.expand_path(File.dirname(__FILE__))
top_dir = File.expand_path(File.join(test_dir, ".."))
$LOAD_PATH.unshift(File.join(top_dir, "bindings", "ruby", ".libs"))
$LOAD_PATH.unshift(File.join(top_dir, "bindings", "ruby"))
$LOAD_PATH.unshift(File.join(test_dir))

require 'tomoe-spec-utils'

ARGV.unshift("--diff=unified")
ARGV.concat(Dir.glob(File.join(test_dir, "*_spec.rb")))
Spec::Runner::CommandLine.run(ARGV, STDERR, STDOUT, true, true)
