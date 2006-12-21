require 'tomoe-spec-utils'

dict_module_type = TomoeSpecUtils::Config.dict_module_type
context "Tomoe::Dict(#{dict_module_type})" do
  read_only_dictionaries = %w(unihan)
  if read_only_dictionaries.include?(dict_module_type)
    puts "#{dict_module_type} is read-only dictionary. skip tests."
    break
  end

  setup do
    setup_dict_file
    @original = Tomoe::DictXML.new("filename" => @dict_file.path)
  end

  specify "should load" do
    make_temporary_dict(@original) do |dict|
      a = dict[@utf8]
      a.writing.strokes.should == @strokes
    end
  end

  specify "should register/unregister" do
    make_temporary_dict(@original) do |dict|
      prev = dict.search(Tomoe::Query.new).collect do |cand|
        cand.char.utf8
      end

      char = Tomoe::Char.new
      char.utf8 = "か"
      dict.register(char).should == true
      dict["か"].should == char
      dict.search(Tomoe::Query.new).collect do |cand|
        cand.char.utf8
      end.sort.should == ["か", *prev].sort
      dict.unregister("か").should == true
      dict["か"].should_nil

      dict.search(Tomoe::Query.new).collect do |cand|
        cand.char.utf8
      end.should == prev
    end
  end

  specify "should register/unregister PUA character" do
    make_temporary_dict(@original) do |dict|
      pua_start = Tomoe::Char::PRIVATE_USE_AREA_START

      utf8_to_ucs4(dict.available_private_utf8).should == pua_start

      char1 = Tomoe::Char.new
      dict.register(char1).should == true
      char1.utf8.should == ucs4_to_utf8(pua_start)
      utf8_to_ucs4(dict.available_private_utf8).should == pua_start + 1

      char2 = Tomoe::Char.new
      dict.register(char2).should == true
      char2.utf8.should == ucs4_to_utf8(pua_start + 1)
      utf8_to_ucs4(dict.available_private_utf8).should == pua_start + 2

      dict[ucs4_to_utf8(pua_start)].should == char1
      dict[ucs4_to_utf8(pua_start + 1)].should == char2
      dict.unregister(char1.utf8).should == true
      dict[ucs4_to_utf8(pua_start)].should_nil
      dict[ucs4_to_utf8(pua_start + 1)].should == char2

      char3 = Tomoe::Char.new
      dict.register(char3).should == true
      char3.utf8.should == ucs4_to_utf8(pua_start + 2)
      utf8_to_ucs4(dict.available_private_utf8).should == pua_start + 3
    end
  end

  def setup_strokes
    @strokes = [
                [
                 [18, 19],
                 [83, 22]
                ],
                [
                 [49, 3],
                 [48, 67],
                 [60, 84],
                ],
                [
                 [74, 34],
                 [49, 76],
                 [27, 80],
                 [17, 68],
                 [28, 49],
                 [60, 46],
                 [80, 57],
                 [82, 74],
                 [76, 83],
                ],
               ]
  end

  def setup_dict_file
    @dict_file = Tempfile.new("tomoe-dict-xml")
    @utf8 = "あ"

    setup_strokes
    strokes_xml = "    <strokes>\n"
    @strokes.each do |stroke|
      strokes_xml << "      <stroke>\n"
      stroke.each do |x, y|
        strokes_xml << "        <point x=\"#{x}\" y=\"#{y}\"/>\n"
      end
      strokes_xml << "      </stroke>\n"
    end
    strokes_xml << "    </strokes>"

    @character_xml = <<-EOC
  <character>
    <utf8>#{@utf8}</utf8>
#{strokes_xml}
  </character>
EOC

    @dict_content = <<-EOX
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE dictionary SYSTEM "tomoe-dict.dtd">
<dictionary>
#{@character_xml}
</dictionary>
EOX
    set_content(@dict_file, @dict_content)
  end

  def setup_est_draft_file
    @est_draft_file = Tempfile.new("tomoe-dict-est-draft")
    @est_draft_content = <<-EOC
@uri=font:#{@utf8}
utf8=#{@utf8}

#{@character_xml.collect {|line| "\t#{line}"}}
EOC
    set_content(@est_draft_file, @est_draft_content)
  end

  def set_content(file, content)
    file.open
    file.truncate(0)
    file.rewind
    file.print(content)
    file.close

    file.open
    file.read.should == content
    file.close
  end

  def truncate_content(file)
    file.set_content("")
  end

  def content(file)
    file.open
    file.read
  ensure
    file.close
  end
end
