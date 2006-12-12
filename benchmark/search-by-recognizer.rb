require "benchmark"
require 'tempfile'

benchmark_dir = File.expand_path(File.dirname(__FILE__))
top_dir = File.expand_path(File.join(benchmark_dir, ".."))
test_dir = File.join(top_dir, "test")
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby", ".libs"))
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby"))
$LOAD_PATH.unshift(File.join(test_dir))

require 'tomoe-spec-utils'

data_dir = TomoeSpecUtils::Config.data_dir
use_est = false
if use_est
  database_name = File.join(data_dir, "handwriting")
  dict = Tomoe::Dict::Est.new("database_name" => database_name,
                              "editable" => false)
else
  filename = File.join(data_dir, "handwriting.xml")
  dict = Tomoe::Dict::XML.new("filename" => filename,
                              "editable" => false)
end
recognizer = Tomoe::Recognizer::Simple.new("dictionary" => dict)
context = Tomoe::Context.new("recognizer" => recognizer)
config_file = TomoeSpecUtils::Config.make_config_file
context.load_config(config_file.path)

n = 10
Benchmark.bmbm do |x|
  TomoeSpecUtils::Config.test_data_files.sort.each do |file|
    expected, writing = TomoeSpecUtils::TestData.parse(file)
    query = Tomoe::Query.new
    query.writing = writing
    base = File.basename(file)
    x.report(base) do
      failed = false
      n.times do
	actual = context.search(query).collect {|cand| cand.char.utf8}
        failed = true if actual != expected
      end
      p "search result doesn't match #{base}" if failed
    end
    GC.start
  end
end
