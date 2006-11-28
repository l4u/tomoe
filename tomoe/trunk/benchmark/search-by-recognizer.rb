require "benchmark"
require 'tempfile'

benchmark_dir = File.expand_path(File.dirname(__FILE__))
top_dir = File.expand_path(File.join(benchmark_dir, ".."))
test_dir = File.join(top_dir, "test")
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby", ".libs"))
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby"))
$LOAD_PATH.unshift(File.join(test_dir))

require 'tomoe-spec-utils'

context = Tomoe::Context.new
config_file = TomoeSpecUtils::Config.make_config_file
context.load_config(config_file.path)
context.load_recognizer(TomoeSpecUtils::Config.recognizer_dir, "simple")

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
	actual = context.search(query).collect {|cand| cand.character.utf8}
        failed = true if actual != expected
      end
      p "search result doesn't match #{base}" if failed
    end
    GC.start
  end
end