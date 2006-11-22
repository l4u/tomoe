require 'tomoe-spec-utils'

context "Tomoe::Context" do
  inherit TomoeSpecBase
  include TomoeSpecUtils

  setup do
    @context = Tomoe::Context.new
    test_dir = File.expand_path(File.join(File.dirname(__FILE__)))
    @context.load_config(@config_file.path)
  end

  def numbers_to_point(str)
    point = str.split.collect {|x| Integer(x)}
    raise ArgumentError if point.size != 2
    point
  end

  specify "Search by strokes" do
    Dir.glob(File.join(@test_data_dir, "*.data")).each do |file|
      base = File.basename(file)
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
      cands = @context.search_by_strokes(writing)

      [base, cands.collect {|cand| cand.character.code}].should == expected
    end
  end

  specify "Search by reading" do
    cands = @context.search_by_reading("せい")
    cands.collect {|cand| cand.character.code}.should == ["脊", "背", "汐"]
  end
end
