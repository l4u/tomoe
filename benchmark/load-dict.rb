require "benchmark"

benchmark_dir = File.expand_path(File.dirname(__FILE__))
top_dir = File.expand_path(File.join(benchmark_dir, ".."))
test_dir = File.join(top_dir, "test")
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby", ".libs"))
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby"))
$LOAD_PATH.unshift(File.join(test_dir))

require "tomoe-spec-utils"

n = 10
Benchmark.bmbm do |x|
  TomoeSpecUtils::Config.dictionaries.sort.each do |dictionary|
    x.report(File.basename(dictionary)) do
      n.times {Tomoe::Dict.new("xml",
                               "filename" => dictionary,
                               "editable" => false)}
    end
    GC.start
  end
end
