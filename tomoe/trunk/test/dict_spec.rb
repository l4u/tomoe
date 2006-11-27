require 'tomoe-spec-utils'

context "Tomoe::Context" do
  setup do
    @dict_file = Tempfile.new("tomoe-dict")
    @dict_content = <<-EOX
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE tomoe_dictionary SYSTEM "tomoe-dict.dtd">
<dictionary>
  <character>
    <code-point>あ</code-point>
    <strokes>
      <stroke>
        <point x="54" y="58"/>
        <point x="249" y="68"/>
      </stroke>
      <stroke>
        <point x="147" y="10"/>
        <point x="145" y="201"/>
        <point x="182" y="252"/>
      </stroke>
      <stroke>
        <point x="224" y="103"/>
        <point x="149" y="230"/>
        <point x="82" y="240"/>
        <point x="53" y="204"/>
        <point x="86" y="149"/>
        <point x="182" y="139"/>
        <point x="240" y="172"/>
        <point x="248" y="224"/>
        <point x="228" y="250"/>
      </stroke>
    </strokes>
  </character>
</dictionary>
EOX
    set_content(@dict_content)
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
