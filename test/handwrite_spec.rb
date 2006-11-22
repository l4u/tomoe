require 'tomoe-spec-utils'

context "Tomoe::Glyph" do
  include TomoeSpecUtils

  specify "should each all strokes" do
    glyph = Tomoe::Glyph.new
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
      glyph.move_to(*first_point)
      rest_points.each do |x, y|
        glyph.line_to(x, y)
      end
    end

    glyph.collect.should == strokes
  end
end
