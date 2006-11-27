require 'tomoe-spec-utils'

context "Tomoe::Context" do
  setup do
    @dict_file = Tempfile.new("tomoe-dict")
    @utf8 = "あ"
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

    strokes_xml = "    <strokes>\n"
    @strokes.each do |stroke|
      strokes_xml << "      <stroke>\n"
      stroke.each do |x, y|
        strokes_xml << "        <point x=\"#{x}\" y=\"#{y}\"/>\n"
      end
      strokes_xml << "      </stroke>\n"
    end
    strokes_xml << "    </strokes>"

    @dict_content = <<-EOX
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE dictionary SYSTEM "tomoe-dict.dtd">
<dictionary>
  <character>
    <utf8>#{@utf8}</utf8>
#{strokes_xml}
  </character>
</dictionary>
EOX
    set_content(@dict_content)
  end

  specify "should load" do
    dict = Tomoe::Dict.new(@dict_file.path, true)
    a = dict[@utf8]
    a.writing.strokes.should == @strokes
  end

  specify "should load and save" do
    dict = Tomoe::Dict.new(@dict_file.path, true)
    truncate_content
    dict.save
    content.should == @dict_content
  end

  def set_content(content)
    @dict_file.open
    @dict_file.truncate(0)
    @dict_file.rewind
    @dict_file.print(content)
    @dict_file.close

    @dict_file.open
    @dict_file.read.should == content
    @dict_file.close
  end

  def truncate_content
    set_content("")
  end

  def content
    @dict_file.open
    @dict_file.read
  ensure
    @dict_file.close
  end
end
