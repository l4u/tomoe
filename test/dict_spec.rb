require 'tomoe-spec-utils'

context "Tomoe::Context" do
  setup do
    @dict_file = Tempfile.new("tomoe-dict")
    @dict_content = <<-EOX
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE tomoe_dictionary SYSTEM "tomoe-dict.dtd">
<tomoe_dictionary name="TOMOE Strokelist Dictionary">
  <character>
    <literal>あ</literal>
    <strokelist>
      <s>(54 58) (249 68) </s>
      <s>(147 10) (145 201) (182 252) </s>
      <s>(224 103) (149 230) (82 240) (53 204) (86 149) (182 139) (240 172) (248 224) (228 250) </s>
    </strokelist>
  </character>
</tomoe_dictionary>
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
