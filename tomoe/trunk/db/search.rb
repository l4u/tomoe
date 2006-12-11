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
  TomoeSpecUtils::Unicode.utf8_to_ucs4(utf8)
end

def ucs4_to_utf8(ucs4)
  TomoeSpecUtils::Unicode.ucs4_to_utf8(ucs4)
end

$LOAD_PATH.unshift(File.join(base, "lib"))

query = Tomoe::Query.new
query.min_n_strokes = 0
query.max_n_strokes = 10

dict = Tomoe::Dict.new("unihan", {})
cands = dict.search(Tomoe::Query.new).find_all do |cand|
  n_strokes = cand.char.n_strokes
  0 <= n_strokes and
    query.min_n_strokes <= n_strokes and
    n_strokes <= query.max_n_strokes
end

puts cands.size
cands.each do |cand|
  char = cand.char
  unless char.readings.empty?
    p [utf8_to_ucs4(char.utf8), char.readings.collect {|x| [x.type, x.reading]}]
  end
  p [utf8_to_ucs4(char.utf8), char.radicals] unless char.radicals.empty?
  p [utf8_to_ucs4(char.utf8), char.collect] unless char.collect.empty?
end

ActiveRecord::Base.logger.level = Logger::DEBUG
chars = Char.find(:all,
                  :conditions => ["? <= n_strokes AND n_strokes <= ?",
                                  query.min_n_strokes,
                                  query.max_n_strokes],
                  :include => [:readings, :radicals])
ActiveRecord::Base.logger.level = Logger::INFO
puts chars.size

puts Reading.count
puts Radical.count

chars.each do |char|
  p [utf8_to_ucs4(char.utf8), char.readings] unless char.readings.empty?
  p [utf8_to_ucs4(char.utf8), char.radicals] unless char.radicals.empty?
  unless char.meta_data.empty?
    p [utf8_to_ucs4(char.utf8), char.meta_data.collect {|x| [x.key, x.value]}]
  end
end
