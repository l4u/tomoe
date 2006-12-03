require 'tomoe-spec-utils'

context "Tomoe::Dict" do
  setup do
    setup_dict_file
    setup_est_draft_file
  end

  specify "should load XML dictionary" do
    dict = Tomoe::Dict.new("xml",
                           "filename" => @dict_file.path,
                           "editable" => true)
    a = dict[@utf8]
    a.writing.strokes.should == @strokes
  end

  specify "should make Hyper Estraier dictionary" do
    begin
      est_db = File.join(tmp_dir, "est")

      `estcmd create #{est_db.dump}`
      `estcmd put #{est_db.dump} #{@est_draft_file.path.dump}`

      dict = Tomoe::Dict.new("est",
                             "database_name" => est_db,
                             "editable" => true)
      return if dict.nil?
      a = dict[@utf8]
      a.writing.strokes.should == @strokes
    ensure
      FileUtils.rm_rf(est_db)
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
