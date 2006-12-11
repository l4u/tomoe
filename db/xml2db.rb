#!/usr/bin/env ruby

top = File.expand_path(File.join(File.dirname(__FILE__), ".."))
base = File.join(top, File.dirname(__FILE__))
require File.join(base, "init")

ActiveRecord::Base.logger.level = Logger::INFO

$LOAD_PATH.unshift(File.join(top, "test"))
$LOAD_PATH.unshift(File.join(top, "ext", "ruby", ".libs"))
$LOAD_PATH.unshift(File.join(top, "ext", "ruby"))

require 'tomoe-spec-utils'

def utf8_to_ucs4(utf8)
  if utf8
    TomoeSpecUtils::Unicode.utf8_to_ucs4(utf8)
  else
    nil
  end
end


$LOAD_PATH.unshift(File.join(base, "lib"))

migrate = File.join(base, "migrate.rb")
print "purging DB..."
$stdout.flush
`#{migrate} 0`
`#{migrate}`
puts "done."

dict = Tomoe::Dict.new("xml",
                       "filename" => TomoeSpecUtils::Config.dictionaries.first,
                       "editable" => false)
cands = dict.search(Tomoe::Query.new)
puts "dict size: #{cands.size}"

print "converting..."
$stdout.flush
$KCODE = "u"
cands.each_with_index do |cand, i|
  char = cand.char
  Char.new(:utf8 => char.utf8,
           :n_strokes => char.n_strokes < 0 ? nil : char.n_strokes,
           :variant => char.variant).save!
  char.readings.each do |reading|
    Reading.new(:utf8 => char.utf8,
                :reading_type => reading.type.to_i,
                :reading => reading.reading).save!
  end
  char.radicals.each do |radical|
    Radical.new(:utf8 => char.utf8,
                :radical_utf8 => radical).save!
  end
  char.each do |key, value|
    MetaDatum.new(:utf8 => char.utf8,
                  :key => key,
                  :value => value).save!
  end
  if (i % 1000).zero? and !i.zero?
    print "#{i}."
    $stdout.flush
  end
end
puts "done."

puts "converted characters: #{Char.count}"
