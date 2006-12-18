require 'tempfile'
require 'fileutils'
require 'yaml'
require 'uconv'

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

    class ContextRunner
      def run(exit_when_done)
        @reporter.start(number_of_specs)
        begin
          @contexts.each do |context|
            context.run(@reporter, @dry_run)
          end
        rescue Interrupt
        ensure
          @reporter.end
        end
        failure_count = @reporter.dump
        if(exit_when_done)
          exit_code = (failure_count == 0) ? 0 : 1
          exit(exit_code)
        end
      end
    end
  end
end

module TomoeSpecUtils
  def self.included(base)
    base.class_eval do
      include Base
      include Path
      include Config
      include Dictionary
      include Unicode
    end
  end

  module Base
    def setup_context
    end

    def teardown_context
    end
  end

  module Path
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

    def db_dir
      File.join(top_dir, "db")
    end

    def db_config_file
      File.join(db_dir, "config.yml")
    end

    def test_data_files
      Dir.glob(File.join(test_data_dir, "*.data"))
    end

    def dictionary
      File.join(data_dir, "kanjidic2.xml")
    end
  end

  module Config
    extend Path

    module_function
    def db_config_for_active_record(type=nil)
      YAML.load(File.read(db_config_file))[type || ENV["TOMOE_ENV"] || "test"]
    end

    def db_config(type=nil)
      config = db_config_for_active_record(type)
      config.delete("adapter")
      config.delete("encoding")
      config["user"] = config.delete("username") if config["username"]
      config
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

    def dict_module_type
      ENV["TOMOE_DICT_MODULE"] || "xml"
    end

    def make_config_file(dict_type=nil)
      dict_type ||= dict_module_type
      name ||= "tomoe-#{dict_type}"
      config_file = Tempfile.new(name)
      config_file.open
      config_file.puts(<<-EOC)
[config]
use_system_dictionaries = false
EOC

      config_maker = "make_config_file_for_#{dict_type}"
      unless respond_to?(config_maker, true)
        raise "unknown dictionary type: #{dict_type}"
      end
      config_file.puts(send(config_maker))

      config_file.close
      config_file
    end

    def make_config_file_for_unihan
      <<-EOC
[unihan-dictionary]
type = unihan
EOC
    end

    def make_config_file_for_est
      <<-EOC
[#{File.basename(dictionary)}-dictionary]
type = est
name = #{File.basename(dictionary)}
database = #{dictionary.sub(/\.xml$/, '')}
EOC
    end

    def make_config_file_for_xml
      <<-EOC
[#{File.basename(dictionary)}-dictionary]
type = xml
file = #{dictionary}
EOC
    end

    def make_config_file_for_mysql
      config = <<-EOC
[mysql-dictionary]
type = mysql
EOC
      db_config.each do |key, value|
        config << "#{key} = #{value}\n"
      end
      config
    end
  end

  module Dictionary
    module_function
    def make_dict(dict_type=nil, config=nil)
      dict_type ||= dict_module_type
      dict_maker = "make_dict_#{dict_type}"
      unless respond_to?(dict_maker, true)
        raise "unknown dictionary type: #{dict_type}"
      end
      send(dict_maker, config)
    end

    def make_temporary_dict(original, dict_type=nil, config=nil, &block)
      dict_type ||= dict_module_type
      temporary_dict_maker = "make_temporary_dict_#{dict_type}"
      unless respond_to?(temporary_dict_maker, true)
        raise "unknown dictionary type: #{dict_type}"
      end
      send(temporary_dict_maker, config) do |dict|
        original.search(Tomoe::Query.new).each do |cand|
          dict.register(cand.char)
        end
        block.call(dict)
      end
    end

    def make_dict_unihan(config=nil)
      check_dict_module_availability("Unihan")
      Tomoe::DictUnihan.new(config || {})
    end

    def make_dict_xml(config=nil)
      check_dict_module_availability("XML")
      config ||= {}
      config = config.dup
      config["filename"] ||= dictionary
      Tomoe::DictXML.new(config)
    end

    def make_temporary_dict_xml(config=nil)
      check_dict_module_availability("XML")
      dict = nil
      begin
        tmp_dict_dir = File.join(tmp_dir, "dict")
        FileUtils.mkdir_p(tmp_dict_dir)
        dict_file = File.join(tmp_dict_dir, "dict.xml")
        dict = Tomoe::DictXML.new("filename" => dict_file, "editable" => true)
        yield dict
      ensure
        dict.flush if dict
        FileUtils.rm_rf(tmp_dict_dir)
      end
    end

    def make_dict_est(config=nil)
      check_dict_module_availability("Est")
      config ||= {}
      config = config.dup
      config["database"] ||= dictionary.sub(/\.xml/, '')
      config["editable"] = true unless config.has_key?("editable")
      Tomoe::DictEst.new(config)
    end

    def make_temporary_dict_est(config=nil)
      check_dict_module_availability("Est")
      begin
        tmp_dict_dir = File.join(tmp_dir, "est")
        yield Tomoe::DictEst.new("database" => tmp_dict_dir, "editable" => true)
      ensure
        FileUtils.rm_rf(tmp_dict_dir)
      end
    end

    def make_dict_mysql(config=nil)
      check_dict_module_availability("MySQL")
      config ||= db_config
      config = config.dup
      Tomoe::DictMySQL.new(config)
    end

    def make_temporary_dict_mysql(config=nil)
      check_dict_module_availability("MySQL")
      sql_purge("temp")
      yield Tomoe::DictMySQL.new(db_config("temp"))
    end

    def check_dict_module_availability(type)
      unless Tomoe.const_defined?("Dict#{type}")
        raise "Tomoe doesn't support the dictionary type: #{type}"
      end
    end

    def sql_migrate(type=nil, version=nil)
      migrate = File.join(db_dir, "migrate.rb")
      tomoe_env = ENV["TOMOE_ENV"]
      ENV["TOMOE_ENV"] = type if type
      unless `#{migrate} #{version}`
        message = "failed to migrate"
        message << " to #{version}" if version
        raise message
      end
    ensure
      ENV["TOMOE_ENV"] = tomoe_env
    end

    def sql_purge(type=nil)
      sql_migrate(type, 0)
      sql_migrate(type)
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

  module Unicode
    module_function
    def ucs4_to_utf8(ucs4)
      Uconv.u4tou8([ucs4].pack("I*"))
    end

    def utf8_to_ucs4(utf8)
      Uconv.u8tou4(utf8).unpack("I*")[0]
    end
  end
end

ENV["TOMOE_DICT_MODULE_DIR"] ||= TomoeSpecUtils::Path.dict_dir
ENV["TOMOE_RECOGNIZER_MODULE_DIR"] ||= TomoeSpecUtils::Path.recognizer_dir

require 'tomoe'
