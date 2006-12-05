require 'tomoe-spec-utils'

context "Tomoe::Context" do
  @@context = nil
  def context
    return @@context if @@context
    dict = Tomoe::Dict.new("xml",
                           "filename" => File.join(data_dir, "handwriting.xml"),
                           "editable" => false)
    recognizer = Tomoe::Recognizer.new("simple", dict)
    @@context = Tomoe::Context.new("recognizer" => recognizer)
    @@context.load_config(@config_file.path)
    @@context
  end

  TomoeSpecUtils::Config.test_data_files.each do |file|
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
    user_dict_file = Tempfile.new("tomoe-user-dict")
    user_dict_file.open
    user_dict_file.puts(<<-EOD)
<?xml version ="1.0" encoding="UTF-8"?>
<!DOCTYPE dictionary SYSTEM "#{File.join(data_dir, 'tomoe-dict.dtd')}">
<dictionary name="User dictionary">
</dictionary>
EOD
    user_dict_file.close

    config_file = Tempfile.new("tomoe-context")
    config_file.open
    config_file.puts(<<-EOC)
[config]
use_system_dictionaries = false
user_dictionary = user

[user-dictionary]
type = xml
file = #{user_dict_file.path}
EOC
    config_file.close

    context = Tomoe::Context.new()
    context.load_config(config_file.path)

    context.search(Tomoe::Query.new).should_be_empty

    char = Tomoe::Char.new
    char.utf8 = "あ"
    context.register(char).should
    context.search(Tomoe::Query.new).collect do |cand|
      cand.char.utf8
    end.should == [char.utf8]

    context.unregister(char.utf8).should
    context.search(Tomoe::Query.new).should_be_empty
  end
end
