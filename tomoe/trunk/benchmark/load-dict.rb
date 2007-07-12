require "benchmark"

benchmark_dir = File.expand_path(File.dirname(__FILE__))
top_dir = File.expand_path(File.join(benchmark_dir, ".."))
test_dir = File.join(top_dir, "test")
$LOAD_PATH.unshift(File.join(top_dir, "bindings", "ruby", ".libs"))
$LOAD_PATH.unshift(File.join(top_dir, "bindings", "ruby"))
$LOAD_PATH.unshift(File.join(test_dir))

require "tomoe-test-utils"

n = 3
Benchmark.bmbm do |x|
  if ARGV.length == 1
    binary = ARGV[0] + ".bin"

    x.report(File.basename(binary)) do
      n.times {Tomoe::DictBinary.new("filename" => binary,
                                     "editable" => false)}
    end

    dictionary = ARGV[0] + ".xml"
  else
    dictionary = TomoeTestUtils::Path.dictionary
  end

  x.report(File.basename(dictionary)) do
    n.times {Tomoe::DictXML.new("filename" => dictionary,
                                "editable" => false)}
  end

  if Tomoe::Dict.exist?("Unihan")
    x.report("Unihan") do
      n.times {Tomoe::DictUnihan.new}
    end
  end
end
