require 'tomoe-spec-utils'

context "Tomoe::Char" do
  specify "should not dump any XML without data" do
    char = Tomoe::Char.new
    char.to_xml.should_empty
  end

  specify "should dump XML with code-point" do
    char = Tomoe::Char.new
    char.utf8 = "あ"
    xml = "  <character>\n"
    xml << "    <utf8>あ</utf8>\n"
    xml << "  </character>\n"
    char.to_xml.should == xml
  end

  specify "should load from dumped XML" do
    char = Tomoe::Char.new
    char.utf8 = "あ"
    new_char = Tomoe::Char.new(char.to_xml)
    new_char.utf8.should == char.utf8
  end

  specify "should set/get n_strokes" do
    char = Tomoe::Char.new
    char.n_strokes.should == -1

    char.n_strokes = 5
    char.n_strokes.should == 5
  end

  specify "should set/get variant" do
    variant = "異"

    char = Tomoe::Char.new
    char.variant.should_nil

    char.variant = variant
    char.variant.should == variant
  end

  specify "should set/get writing" do
    strokes = [[[0, 0], [10, 10]]]
    writing = Tomoe::Writing.new
    writing.move_to(*strokes[0][0])
    writing.line_to(*strokes[0][1])
    writing.strokes.should == strokes

    char = Tomoe::Char.new
    char.writing.should_nil

    char.writing = writing
    char.writing.strokes.should == strokes
  end

  specify "should add/get reading" do
    reading_a = Tomoe::Reading.new(Tomoe::Reading::JA_ON, "あ")

    reading_kai = Tomoe::Reading.new(Tomoe::Reading::JA_KUN, "カイ")

    char = Tomoe::Char.new
    char.readings.should_empty

    char.add_reading(reading_a)
    char.readings.collect do |reading|
      reading.reading
    end.sort.should == [reading_a.reading]

    char.add_reading(reading_kai)
    char.readings.collect do |reading|
      reading.reading
    end.sort.should == [reading_a.reading, reading_kai.reading].sort
  end

  specify "should add/get radical" do
    radical1 = "木"
    radical2 = "水"

    char = Tomoe::Char.new
    char.radicals.should_empty

    char.add_radical(radical1)
    char.radicals.sort.should == [radical1]

    char.add_radical(radical2)
    char.radicals.sort.should == [radical1, radical2].sort
  end

  specify "should be comparable" do
    char1 = Tomoe::Char.new
    char1.utf8 = "あ"
    char2 = Tomoe::Char.new
    char2.utf8 = "あ"
    char1.should == char2
  end
end
