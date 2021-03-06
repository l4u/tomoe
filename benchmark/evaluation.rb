#!/usr/bin/env ruby

benchmark_dir = File.expand_path(File.dirname(__FILE__))
top_dir = File.expand_path(File.join(benchmark_dir, ".."))
test_dir = File.join(top_dir, "test")
$LOAD_PATH.unshift(File.join(top_dir, "bindings", "ruby", ".libs"))
$LOAD_PATH.unshift(File.join(top_dir, "bindings", "ruby"))
$LOAD_PATH.unshift(File.join(test_dir))

require 'tomoe-test-utils'

class Evaluation
  include TomoeTestUtils::Path

  def initialize(recognizer_class, set)
    @recognizer_class = recognizer_class

    data_set_dir = File.join(benchmark_data_dir, set)
    @xml_files = Dir.glob(File.join(data_set_dir, "*.xml"))
    @n_files = @xml_files.length

    @results = {}
    @match1 = []
    @match5 = []
    @match10 = []
  end

  def run
    dict = Tomoe::DictXML.new("filename" => handwriting_dictionary("ja"),
                              "editable" => false)
    recognizer = @recognizer_class.new("dictionary" => dict)
    context = Tomoe::Context.new("recognizer" => recognizer)
    context.load_config()
    query = Tomoe::Query.new

    @xml_files.each do |xml_file|
      char = Tomoe::Char.new(IO.read(xml_file))
      char_utf8 = char.utf8
      query.writing = char.writing
      candidates = context.search(query).map {|c| c.char.utf8}
      ten_candidates = candidates[0...10]

      @results[char_utf8] = candidates

      @match1 << char_utf8 if ten_candidates.first == char_utf8
      @match5 << char_utf8 if ten_candidates[0...5].include?(char_utf8)
      @match10 << char_utf8 if ten_candidates.include?(char_utf8)
    end
  end

  def first_match_accuracy
    @match1.length.to_f / @n_files * 100
  end

  def fith_match_accuracy
    @match5.length.to_f / @n_files * 100
  end

  def tenth_match_accuracy
    @match10.length.to_f / @n_files * 100
  end

  def character_details(char_utf8)
    ret = char_utf8 + "\t"
    i = @results[char_utf8].index(char_utf8)
    ret << (i ? "%d" % (i + 1) : "X")
    ret << "\t"
    ret << @results[char_utf8][0...5].join(", ")
  end

  def characters
    @results.keys
  end
end

detailed = ARGV.length >= 1 and ARGV[0] == "-d"
puts "Run with -d for a detailed report" unless detailed


[Tomoe::RecognizerSimple].each do |recognizer|
  sets = ["set1", "set2"]

  match1, match5, match10 = 0, 0, 0

  sets.each do |set|
    puts "#{recognizer.to_s}, #{set}"
    evaluation = Evaluation.new(recognizer, set)
    evaluation.run
    puts "1st match: #{evaluation.first_match_accuracy.to_s}%"
    puts "5th match: #{evaluation.fith_match_accuracy.to_s}%"
    puts "10th match: #{evaluation.tenth_match_accuracy.to_s}%"

    if detailed
      evaluation.characters.each do |char|
        puts evaluation.character_details(char)
      end
    end

    match1 += evaluation.first_match_accuracy
    match5 += evaluation.fith_match_accuracy
    match10 += evaluation.tenth_match_accuracy
  end

  n_sets = sets.length

  puts "#{recognizer.to_s}, overall"
  puts "1st match: #{(match1.to_f / n_sets).to_s}%"
  puts "5th match: #{(match5.to_f / n_sets).to_s}%"
  puts "10th match: #{(match10.to_f / n_sets).to_s}%\n"
end
