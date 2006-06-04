#!/usr/bin/ruby

require 'xml/libxml'

dictfilepath="../data/all.tdic"
newdictfilepath="../data/generated.xml"

doc = XML::Document.new()
doc.root = XML::Node.new('dict')
root = doc.root

open(dictfilepath, "r") do |f|
  while ((line = f.gets) != nil)
    root << character = XML::Node.new('character')

    line.chomp!
    ch = line
    character << literal = XML::Node.new('literal')
    literal << ch

    line = f.gets
    line.chomp!
    stroke_num = line[1 .. line.length].to_i

    character << strokelist = XML::Node.new('strokelist')
    strokelist['count'] = stroke_num.to_s()

    strokes = []
    for i in 1 .. stroke_num
      line = f.gets
      line.chomp!
      vector_num = line[0 .. line.index(" ")].to_i
      strokelist << stroke = XML::Node.new('s')
      stroke['count'] = vector_num.to_s()
      stroke << line[line.index(" ") + 1 .. line.length]
    end
    empty_line = f.gets
  end
end
doc.save(newdictfilepath, true)
