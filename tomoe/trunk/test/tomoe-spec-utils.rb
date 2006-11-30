require 'tempfile'
require 'fileutils'

require 'tomoe'

$KCODE = "u"

module TomoeSpecSetup
  def before_context_eval
    super
    @context_eval_module.class_eval do
      include TomoeSpecUtils

      setup do
        setup_context
      end

      teardown do
        teardown_context
      end
    end
  end
end

module Spec
  module Runner
    class Context
      include TomoeSpecSetup
    end

    module Formatter
      class ProgressBarFormatter
        def format_backtrace(backtrace)
          super(backtrace.collect {|bt| bt.sub(/\A([^:]+:\d+)$/, '\\1:')})
        end
      end
    end
  end
end

module TomoeSpecUtils
  def self.included(base)
    base.class_eval do
      include Base
      include Config
    end
  end

  module Base
    def setup_context
    end

    def teardown_context
    end
  end

  module Config
    module_function
    def base_dir
      File.expand_path(File.dirname(__FILE__))
    end

    def tmp_dir
      File.join(base_dir, "tmp")
    end

    def top_dir
      File.expand_path(File.join(base_dir, ".."))
    end

    def data_dir
      File.join(top_dir, "data")
    end

    def test_data_dir
      File.join(base_dir, "data")
    end

    def module_dir
      File.join(top_dir, "module")
    end

    def recognizer_dir
      File.join(module_dir, "recognizer", ".libs")
    end

    def dict_dir
      File.join(module_dir, "dict", ".libs")
    end

    def setup_context
      super
      @config_file = make_config_file
      FileUtils.mkdir_p(tmp_dir)
    end

    def teardown_context
      super
      FileUtils.rm_rf(tmp_dir)
    end

    def dictionaries
      %w(kanjidic2.xml all.xml).collect do |xml|
        File.join(data_dir, xml)
      end
    end

    def test_data_files
      Dir.glob(File.join(test_data_dir, "*.data"))
    end

    def make_config_file(name=nil, use_est=false)
      name ||= "tomoe"
      config_file = Tempfile.new(name)
      config_file.open
      config_file.puts(<<-EOC)
[config]
use_system_dictionaries = false
EOC
      dictionaries.each_with_index do |dictionary, i|
        if use_est
          config_file.puts(<<-EOC)
[#{File.basename(dictionary)}-dictionary]
type = est
name = #{File.basename(dictionary)}
database = #{dictionary.sub(/\.xml$/, '')}
#{(i % 2).zero? ? 'use = true' : ''}
EOC
        else
          config_file.puts(<<-EOC)
[#{File.basename(dictionary)}-dictionary]
type = xml
file = #{dictionary}
#{(i % 2).zero? ? 'use = true' : ''}
EOC
        end
      end
      config_file.close
      config_file
    end
  end

  module TestData
    module_function
    def parse(file)
      expected = nil
      writing = Tomoe::Writing.new
      File.open(file) do |f|
        expected = f.gets.split
        f.each do |line|
          next if /\A\s*\z/ =~ line
          begin
            first_point, *rest_points = line.split(/,/)
            numbered_first_point = numbers_to_point(first_point)
            writing.move_to(*numbered_first_point)
            rest_points.each do |point|
              writing.line_to(*numbers_to_point(point))
            end
          rescue ArgumentError
            raise "invalid format in #{file} at #{f.lineno}: #{line}"
          end
        end
      end
      [expected, writing]
    end

    def numbers_to_point(str)
      point = str.split.collect {|x| Integer(x)}
      raise ArgumentError if point.size != 2
      point
    end
  end
end

Tomoe::Dict.unload
Tomoe::Dict.load(TomoeSpecUtils::Config.dict_dir)
Tomoe::Recognizer.unload
Tomoe::Recognizer.load(TomoeSpecUtils::Config.recognizer_dir)
