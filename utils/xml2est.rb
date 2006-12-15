#!/usr/bin/env ruby

benchmark_dir = File.expand_path(File.dirname(__FILE__))
top_dir = File.expand_path(File.join(benchmark_dir, ".."))
test_dir = File.join(top_dir, "test")
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby", ".libs"))
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby"))
$LOAD_PATH.unshift(File.join(test_dir))

require "tomoe-spec-utils"

dictionary = TomoeSpecUtils::Path.dictionary
print "converting #{dictionary}..."
est_db = dictionary.sub(/\.xml$/, "")
xml_dict = Tomoe::Dict.new("xml",
                           "filename" => dictionary,
                           "editable" => false)
est_dict = Tomoe::Dict.new("est",
                           "database" => est_db,
                           "editable" => true)
xml_dict.search(Tomoe::Query.new).each do |cand|
  est_dict.register(cand.char)
end
puts "done."
