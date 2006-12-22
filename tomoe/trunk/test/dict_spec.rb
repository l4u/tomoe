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

  specify "should load successfully" do
    make_temporary_dict(@original) do |dict|
      a = dict[@utf8]
      a.should.not.nil
      a.utf8.should == @utf8
      if dict_module_type == "mysql"
        puts "MySQL backend doesn't support writing"
      else
        a.writing.strokes.should == @strokes
      end
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

  specify "should save/restore meta data" do
    make_temporary_dict(@original) do |dict|
      char = Tomoe::Char.new
      char.utf8 = "か"
      char.should.not.have_meta_data

      char["meta1"] = "value1"
      char["meta2"] = "value2"

      dict.register(char).should == true

      registered_char = dict["か"]
      char.should.have_meta_data
      registered_char["meta1"].should == "value1"
      registered_char["meta2"].should == "value2"
    end
  end

  specify "should support reading search" do
    make_temporary_dict(@original) do |dict|
      char = Tomoe::Char.new
      char.utf8 = "池"
      char.add_reading(Tomoe::Reading.new(Tomoe::Reading::JA_KUN, "いけ"))

      dict.register(char).should == true

      query = Tomoe::Query.new
      query.add_reading(Tomoe::Reading.new(Tomoe::Reading::JA_KUN, "いけ"))
      dict.search(query).collect do |cand|
        cand.char.utf8
      end.should == ["池"]
    end
  end

  specify "should support n_strokes search" do
    make_temporary_dict(@original) do |dict|
      char = Tomoe::Char.new
      char.utf8 = "池"
      char.n_strokes = 6

      dict.register(char).should == true

      query = Tomoe::Query.new
      query.min_n_strokes = 6
      query.max_n_strokes = 6
      dict.search(query).collect do |cand|
        cand.char.utf8
      end.should == ["池"]

      query = Tomoe::Query.new
      query.min_n_strokes = 6
      query.max_n_strokes = 7
      dict.search(query).collect do |cand|
        cand.char.utf8
      end.should == ["池"]

      query = Tomoe::Query.new
      query.min_n_strokes = 5
      query.max_n_strokes = 6
      dict.search(query).collect do |cand|
        cand.char.utf8
      end.should == ["池"]

      query = Tomoe::Query.new
      query.min_n_strokes = 6
      dict.search(query).collect do |cand|
        cand.char.utf8
      end.should == ["池"]

      query = Tomoe::Query.new
      query.max_n_strokes = 6
      dict.search(query).collect do |cand|
        cand.char.utf8
      end.sort.should == ["あ", "池"].sort
    end
  end

  specify "should support radical search" do
    make_temporary_dict(@original) do |dict|
      sanzui = ucs4_to_utf8(27701) # さんずい
      char = Tomoe::Char.new
      char.utf8 = "池"
      char.add_radical(sanzui)

      dict.register(char).should == true

      query = Tomoe::Query.new
      query.add_radical(sanzui)
      dict.search(query).collect do |cand|
        cand.char.utf8
      end.should == ["池"]
    end
  end

  specify "should support variant search" do
    make_temporary_dict(@original) do |dict|
      hashigo_daka = ucs4_to_utf8(39641) # はしご高
      char = Tomoe::Char.new
      char.utf8 = "高"
      char.variant = hashigo_daka

      dict.register(char).should == true

      query = Tomoe::Query.new
      query.variant = hashigo_daka
      dict.search(query).collect do |cand|
        cand.char.utf8
      end.should == ["高"]
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
    <number-of-strokes>#{@strokes.size}</number-of-strokes>
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
n_strokes=#{@strokes.size}

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
