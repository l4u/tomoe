require 'tempfile'

require 'tomoe'

$KCODE = "u"

class TomoeSpecBase
  def setup
    setup_context
  end

  def teardown
    teardown_context
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
    def setup_context
      super
      @base_dir = File.expand_path(File.dirname(__FILE__))
      @top_dir = File.expand_path(File.join(@base_dir, ".."))
      @data_dir = File.join(@top_dir, "data")
      @test_data_dir = File.join(@base_dir, "data")
      @recognizer_dir = File.join(@top_dir, "recognizer", ".libs")
      @config_file = Tempfile.new("tomoe")
      setup_config_file
    end

    def teardown_context
      super
    end

    def setup_config_file
      @config_file.open
      @config_file.puts(<<-EOX)
<?xml version="1.0" standalone="no"?>
<tomoeConfig>
  <dictionary file="#{File.join(@data_dir, "kanjidic2.xml")}"/>
  <dictionary file="#{File.join(@data_dir, "readingtest.xml")}"/>
  <dictionary file="#{File.join(@data_dir, "all.xml")}"/>
</tomoeConfig>
EOX
      @config_file.close
    end
  end
end
