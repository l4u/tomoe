require 'tempfile'

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

    def top_dir
      File.expand_path(File.join(base_dir, ".."))
    end

    def data_dir
      File.join(top_dir, "data")
    end

    def test_data_dir
      File.join(base_dir, "data")
    end

    def recognizer_dir
      File.join(top_dir, "recognizer", ".libs")
    end

    def setup_context
      super
      @config_file = Tempfile.new("tomoe")
      setup_config_file
    end

    def teardown_context
      super
    end

    def setup_config_file
      files = %w(kanjidic2.xml readingtest.xml all.xml)
      names = files.collect {|file| File.join(data_dir, file)}.join(";")
      @config_file.open
      @config_file.puts(<<-EOC)
[config]
use_system_dictionaries = false

[all-dictionary]
type = xml
file = #{File.join(data_dir, "all.xml")}
use = true

[kanjidic2-dictionary]
type = xml
file = #{File.join(data_dir, "kanjidic2.xml")}

EOC
      @config_file.close
    end
  end
end
