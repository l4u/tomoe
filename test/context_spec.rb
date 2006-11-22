require 'tomoe-spec-utils'

context "Tomoe::Context" do
  inherit TomoeSpecBase
  include TomoeSpecUtils

  setup do
    @context = Tomoe::Context.new
    test_dir = File.expand_path(File.join(File.dirname(__FILE__)))
    @context.load_config(@config_file.path)
  end

  specify "Search by strokes" do
    writing = Tomoe::Writing.new
    strokes = [
             [[51, 29], [177, 41]],
             [[99, 65], [219, 77]],
             [[27, 131], [261, 131]],
             [[129, 17], [57, 203]],
             [[111, 71], [219, 173]],
             [[81, 161], [93, 281]],
             [[99, 167], [207, 167], [189, 245]],
             [[99, 227], [189, 227]],
             [[111, 257], [189, 245]],
            ]
    strokes.each do |first_point, *rest_points|
      writing.move_to(*first_point)
      rest_points.each do |x, y|
        writing.line_to(x, y)
      end
    end
    cands = @context.search_by_strokes(writing)

    cands.collect {|cand| cand.character.code}.should == ["春", "屠"]
  end

  specify "Search by reading" do
    cands = @context.search_by_reading("せい")
    cands.collect {|cand| cand.character.code}.should == ["脊", "背", "汐"]
  end
end
