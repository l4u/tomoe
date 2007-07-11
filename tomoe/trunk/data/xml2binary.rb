#!/usr/bin/env ruby

if ARGV.length != 1
    puts "Missing param: xml dictionary file"
    exit!
end

require "tomoe"

print "converting #{ARGV[0]}..."
binary_file = ARGV[0].sub(/\.xml$/, ".bin")
xml_dict = Tomoe::DictXML.new("filename" => ARGV[0],
                              "editable" => false)
binary_dict = Tomoe::DictBinary.new("filename" => binary_file,
                                    "editable" => true)
xml_dict.search(Tomoe::Query.new).each_with_index do |cand, i|
  binary_dict.register(cand.char)
  if (i % 1000).zero? and !i.zero?
    print "#{i}."
    $stdout.flush
  end
end
puts "done."

puts "converted characters: #{binary_dict.search(Tomoe::Query.new).size}"