#!/usr/bin/ruby

require 'xml/libxml'

class Point
  def initialize(x = 0, y = 0)
    @x = x
    @y = y
  end

  attr_accessor :x
  attr_accessor :y

  def to_s
    "(" + @x.to_s + ", " + @y.to_s + ")"
  end
end

class Stroke
  def initialize(points)
    @points = points
  end

  attr_accessor :points

  def to_s
    s = "{"
    for point in @points
      if s != "{"
        s += ", "
      end
      s += point.to_s
    end
    s += "}"
    return s
  end
end

class Letter
  def initialize(character, strokes)
    @character = character
    @strokes = strokes
  end

  attr_accessor :character
  attr_accessor :strokes

  def to_s
    s = "[" + @character + " "
    i = 0
    for stroke in @strokes
      if i != 0
        s += ", "
      end
      s += stroke.to_s
      i += 1
    end
    s += "]"
    return s
  end
end

class Dictionary
  def initialize(dictfilepath="../data/all.xml")
    @letters = load(dictfilepath)
    @letters_by_stroke_count = group_by_stroke_count(@letters)
  end

  attr_accessor :letters
  attr_accessor :letters_by_stroke_count

  def load(dictfilepath)
    letters = []

    doc = XML::Document.file(dictfilepath)
    root = doc.root
    doc.find('//dict/character').each do |node|
      literal = node.find('literal').to_a.first
#      puts "letter #{literal}\n"

      strokes = []
      points = []
      node.find('strokelist/s').each do |stroke|
	line = "#{stroke}\n"
	index1 = 0
	index2 = 0
	points = []
	index1 = line.index("(", index2)
	while index1 != nil
	  index1 += 1
	  index2 = line.index(" ", index1)
	  x = line.slice(index1, index2)
	  index1 = index2 + 1
	  index2 = line.index(")", index1)
	  y = line.slice(index1, index2)
          point = Point.new(x.to_i, y.to_i)
          points.push(point)
	  index1 = line.index("(", index2)
        end
        stroke = Stroke.new(points)
        strokes.push(stroke)
      end
      letter = Letter.new("#{literal}", strokes)
      letters.push(letter)
    end

    return letters
  end

  def group_by_stroke_count(letters)
    letters_by_stroke_count = []
    for letter in letters
      i = letter.strokes.size - 1
      if letters_by_stroke_count[i] == nil
	letters_by_stroke_count[i] = []
      end
      letters_by_stroke_count[i].push(letter)
    end
    return letters_by_stroke_count
  end

  def to_s
    s = ""
    i = 0
    for letter in @letters
      if i != 0
        s += ", "
      end
      s += letter.to_s
      i += 1
    end
    return s
  end

#  def to_s_by_stroke_count
#    s = ""
#    for i in 0 .. @letters_by_stroke_count.size - 1
#      stroke_count = i + 1
#      letters = @letters_by_stroke_count[i]
#      s += stroke_count.to_s + "�"
#      for letter in letters
#	s += " " + letter.character
#      end
#      s += "\n"
#    end
#    return s
#  end

  def get_letter(character)
    for letter in @letters
      if letter.character == character
	return letter
      end
    end
    return nil
  end
end

if __FILE__ == $0
  t1 = Time.now
  d = Dictionary.new
  t2 = Time.now

#  print d.to_s_by_stroke_count
#  print "elapsed time(sec)=" + (t2 - t1).to_s + "\n"
end
