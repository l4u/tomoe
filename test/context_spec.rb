require 'tomoe-spec-utils'

context "Tomoe::Context" do
  @@context = nil
  def context
    return @@context if @@context
    @@context = Tomoe::Context.new
    @@context.load_config(@config_file.path)
    @@context.load_recognizer(recognizer_dir, "simple")
    @@context
  end

  def numbers_to_point(str)
    point = str.split.collect {|x| Integer(x)}
    raise ArgumentError if point.size != 2
    point
  end

  test_data_dir = TomoeSpecUtils::Config.test_data_dir
  Dir.glob(File.join(test_data_dir, "*.data")).each do |file|
    base = File.basename(file)
    specify "Search by strokes for #{base}" do
      expected = [base]
      writing = Tomoe::Writing.new
      File.open(file) do |f|
        expected << f.gets.split
        f.each do |line|
          next if /\A\s*\z/ =~ line
          begin
            first_point, *rest_points = line.split(/,/)
            numbered_first_point = numbers_to_point(first_point)
            writing.move_to(*numbered_first_point)
            rest_points.each do |point|
              writing.line_to(*numbers_to_point(point))
            end
          rescue ArgumentError
            raise "invalid format in #{file} at #{f.lineno}: #{line}"
          end
        end
      end
      query = Tomoe::Query.new
      query.writing = writing
      cands = context.search(query)

      [base, cands.collect {|cand| cand.character.utf8}].should == expected
    end
  end

  specify "Search by reading" do
    query = Tomoe::Query.new
    query.add_reading(Tomoe::Reading.new(Tomoe::READING_KUN, "せい"))
    cands = context.search(query)
    cands.collect {|cand| cand.character.utf8}.should == ["脊", "背", "汐"]
  end
end
