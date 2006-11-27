require 'tomoe-spec-utils'

context "Tomoe::Char" do
  specify "should not dump any XML without data" do
    @char = Tomoe::Char.new
    @char.to_xml.should_empty
  end

  specify "should dump XML with code-point" do
    @char = Tomoe::Char.new
    @char.code = "あ"
    xml = "  <character>\n"
    xml << "    <code-point>あ</code-point>\n"
    xml << "  </character>\n"
    @char.to_xml.should == xml
  end
end
