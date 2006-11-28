require 'tomoe-spec-utils'

context "Tomoe::Context" do
  @@context = nil
  def context
    return @@context if @@context
    @@context = Tomoe::Context.new
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
      actual = cands.collect {|cand| cand.character.utf8}

      [base, actual].should == [base, expected]
    end
  end

  specify "Search by reading" do
    query = Tomoe::Query.new
    query.add_reading(Tomoe::Reading.new(Tomoe::READING_JA_KUN, "せい"))
    cands = context.search(query)
    cands.collect {|cand| cand.character.utf8}.should == ["汐", "背", "脊"]
  end
end
