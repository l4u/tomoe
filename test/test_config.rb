require 'tomoe-spec-utils'

context "Tomoe::Config" do
  def config_file(contents)
    config_file = Tempfile.new("tomoe-context")
    config_file.open
    config_file.puts(contents)
    config_file.close
    config_file
  end

  specify "Get language" do
    file = config_file(<<-EOC)
[config]
language = ja
EOC
    config = Tomoe::Config.new(file.path)
    config.languages.should == ['ja']
  end

  specify "Get languages" do
    file = config_file(<<-EOC)
[config]
languages = ja
EOC
    config = Tomoe::Config.new(file.path)
    config.languages.should == ['ja']

    file = config_file(<<-EOC)
[config]
languages = ja;en;fr
EOC
    config = Tomoe::Config.new(file.path)
    config.languages.should == ['ja', 'en', 'fr']
  end

  specify "both language and languages are specified" do
    file = config_file(<<-EOC)
[config]
language = ja
languages = en;ja;fr
EOC
    config = Tomoe::Config.new(file.path)
    config.languages.should == ['ja']
  end
end
