require 'tomoe-spec-utils'

context "Tomoe::Context" do
  @@context = nil
  def context
    return @@context if @@context
    config = {
      "filename" => File.join(data_dir, "handwriting.xml"),
      "editable" => false,
    }
    dict = Tomoe::DictXML.new(config)
    recognizer = Tomoe::RecognizerSimple.new("dictionary" => dict)
    @@context = Tomoe::Context.new("recognizer" => recognizer)
    @@context.load_config(@config_file.path)
    @@context
  end

  setup do
    setup_user_dict
  end

  def setup_user_dict
    @user_dict_file = Tempfile.new("tomoe-user-dict")
    @user_dict_file.open
    @user_dict_file.puts(<<-EOD)
<?xml version ="1.0" encoding="UTF-8"?>
<!DOCTYPE dictionary SYSTEM "#{File.join(data_dir, 'tomoe-dict.dtd')}">
<dictionary name="User dictionary">
</dictionary>
EOD
    @user_dict_file.close

    @user_dict_config_file = Tempfile.new("tomoe-context")
    @user_dict_config_file.open
    @user_dict_config_file.puts(<<-EOC)
[config]
use_system_dictionaries = false
user_dictionary = user

[user-dictionary]
type = xml
file = #{@user_dict_file.path}
EOC
    @user_dict_config_file.close
  end

  TomoeSpecUtils::Path.test_data_files.each do |file|
    base = File.basename(file)
    specify "Search by strokes for #{base}" do
      expected, writing = TomoeSpecUtils::TestData.parse(file)
      query = Tomoe::Query.new
      query.writing = writing
      cands = context.search(query)
      actual = cands.collect {|cand| cand.char.utf8}

      [base, actual].should == [base, expected]
    end
  end

  specify "Search by reading" do
    query = Tomoe::Query.new
    query.add_reading(Tomoe::Reading.new(Tomoe::READING_JA_KUN, "せい"))
    cands = context.search(query)
    cands.collect {|cand| cand.char.utf8}.sort.should == ["汐", "背", "脊"].sort
  end

  specify "User dictionary" do
    context = Tomoe::Context.new()
    context.load_config(@user_dict_config_file.path)

    context.search(Tomoe::Query.new).should_be_empty

    char = Tomoe::Char.new
    char.utf8 = "あ"
    context.register(char).should == true
    context.search(Tomoe::Query.new).collect do |cand|
      cand.char.utf8
    end.should == [char.utf8]

    context.unregister(char.utf8).should == true
    context.search(Tomoe::Query.new).should_be_empty
  end

  specify "should assign available UTF8 encoded code point " \
          "when a character is registered without UTF8 value" do
    context = Tomoe::Context.new()
    context.load_config(@user_dict_config_file.path)

    n_strokes = 8
    query = Tomoe::Query.new
    query.min_n_strokes = n_strokes
    query.max_n_strokes = n_strokes

    context.search(query).should_be_empty

    char = Tomoe::Char.new
    char.n_strokes = n_strokes
    context.register(char).should == true
    char.utf8.should == ucs4_to_utf8(Tomoe::Char::PRIVATE_USE_AREA_START)

    context.search(query).collect do |cand|
      cand.char.utf8
    end.should == [char.utf8]


    char2 = Tomoe::Char.new
    char2.n_strokes = n_strokes
    context.register(char2).should == true
    char2.utf8.should == ucs4_to_utf8(Tomoe::Char::PRIVATE_USE_AREA_START + 1)

    context.search(query).collect do |cand|
      cand.char.utf8
    end.sort.should == [char.utf8, char2.utf8].sort
  end

  specify "should get character by UTF8" do
    context = Tomoe::Context.new()
    context.load_config(@user_dict_config_file.path)

    context[ucs4_to_utf8(Tomoe::Char::PRIVATE_USE_AREA_START)].should_nil

    char = Tomoe::Char.new
    char.n_strokes = 8
    context.register(char).should == true
    char.utf8.should == ucs4_to_utf8(Tomoe::Char::PRIVATE_USE_AREA_START)

    retrieved_char = context[ucs4_to_utf8(Tomoe::Char::PRIVATE_USE_AREA_START)]
    retrieved_char.n_strokes.should == char.n_strokes
  end
end
