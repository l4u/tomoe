#!/usr/bin/ruby
#
# Copyright (C) 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
# Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA
#
# $Id$
#

class Point
  def initialize(x = 0, y = 0)
    @x = x
    @y = y
  end

  attr_accessor :x
  attr_accessor :y

  def to_s
    "(" + @x.to_s + ", " + @y.to_s + ")"
  end
end

class Metric
  def initialize(a = 0, b = 0, c = 0, d = 0, e = 0, angle = 0)
    @a = a
    @b = b
    @c = c
    @d = d
    @e = e
    @angle = angle
  end

  attr_accessor :a
  attr_accessor :b
  attr_accessor :c
  attr_accessor :d
  attr_accessor :e
  attr_accessor :angle

  def to_s
    return "[a=" + @a.to_s +
	", b=" + @b.to_s +
	", c=" + @c.to_s +
	", d=" + @d.to_s +
	", e=" + @e.to_s +
	", angle=" + @angle.to_s +
	"]"
  end
end

class Stroke
  def initialize(points)
    @points = points
    @metrics = create_metrics
  end

  def create_metrics
    metrics = []
    for i in 0 .. points.size - 1 - 1
      pi = points[i]
      pi1 = points[i + 1]
      a = pi1.x - pi.x
      b = pi1.y - pi.y
      c = pi1.y * pi.x - pi1.x * pi.y
      d = Math.sqrt(a * a + b * b)
      e = a * pi.x + b * pi.y
      angle = Math.atan2(pi1.y - pi.y, pi1.x - pi.x)
      m = Metric.new(a, b, c, d, e, angle)
      metrics.push(m)
    end
    return metrics
  end

  attr_accessor :points
  attr_accessor :metrics

  def to_s
    s = "{"
    for point in @points
      if s != "{"
        s += ", "
      end
      s += point.to_s
    end
    s += "}"
    return s
  end
end

class Letter
  def initialize(character, strokes)
    @character = character
    @strokes = strokes
  end

  attr_accessor :character
  attr_accessor :strokes

  def to_s
    s = "[" + @character + " "
    i = 0
    for stroke in @strokes
      if i != 0
        s += ", "
      end
      s += stroke.to_s
      i += 1
    end
    s += "]"
    return s
  end
end

class Candidate
  def initialize(char_index)
    @char_index = char_index
    @adapted = []
  end

  attr_accessor :char_index
  attr_accessor :adapted

  def clone
    other = Candidate.new(char_index)
    other.adapted = adapted.clone
    return other
  end

  def dup
    return clone
  end

  def to_s
    s = "[c_idx=" + @char_index.to_s + ", adapted="
    i = 0
    for elem in adapted
      if i != 0
        s += ", "
      end
      s += elem.to_s
      i += 1
    end
    s += "]"
    return s
  end
end

class Dictionary
  def initialize(dictfilepath)
    @letters = load(dictfilepath)
  end

  def to_s
    s = ""
    i = 0
    for letter in @letters
      if i != 0
        s += ", "
      end
      s += letter.to_s
      i += 1
    end
    return s
  end

  def candidate_to_s(candidate)
    letter = @letters[candidate.char_index]
    return letter.character
  end

  def candidates_to_s(candidates)
    s = ""
    i = 0
    for candidate in candidates
      if i != 0
        s += ", "
      end
      s += candidate_to_s(candidate)
      i += 1
    end
    return s
  end

  def load(dictfilepath)
    letters = []
    open(dictfilepath, "r") do |f|
      while ((line = f.gets) != nil)
        line.chomp!
        character = line

        line = f.gets
        line.chomp!
        stroke_num = line[1 .. line.length].to_i

        strokes = []
        for i in 1 .. stroke_num
          line = f.gets
          line.chomp!
          point_num, rest = line.split(" ", 2)
          points = []
          rest.scan(/\(([0-9]+) ([0-9]+)\)\s?/) {|x, y|
            point = Point.new(x.to_i, y.to_i)
            points.push(point)
          }
          stroke = Stroke.new(points)
          strokes.push(stroke)
        end

        empty_line = f.gets

        letter = Letter.new(character, strokes)
#print "letter=", letter, "\n"
	letters.push(letter)
      end
    end
    return letters
  end

  def match_char(strokes)
    candidates = []
    for i in 0 .. @letters.size - 1
      if strokes.size <= @letters[i].strokes.size
        candidate = Candidate.new(i)
        candidates.push(candidate)
      end
    end

    for i in 0 .. strokes.size - 1
      verbose_candidates = get_candidates(strokes, i, candidates)
      pre_char = ""
      candidates = []
      adapted_map = {}
      for verbose_candidate in verbose_candidates
        verbose_char_index = verbose_candidate.char_index
        verbose_adapted = verbose_candidate.adapted
        key = verbose_adapted.sort.join(':')
        if (pre_char != verbose_char_index ||
            adapted_map[key] == nil)
            adapted_map[key] = 1
            candidates.push(verbose_candidate)
        end
        pre_char = verbose_char_index
      end
    end
    return candidates
  end

  def sq_len(x, y)
    return x * x + y * y
  end

  def sq_dist(p, q)
    return sq_len(p.x - q.x, p.y - q.y)
  end

  def contains(adapted, j)
    for k in adapted
      if k == j
        return true
      end
    end
    return false
  end

  def match_input_to_dict(i_stroke, d_stroke, limit_length)
    i_pts = i_stroke.points
    i_met = i_stroke.metrics
    i_nop = i_pts.size
    d_pts = d_stroke.points
    d_met = d_stroke.metrics
    d_nop = d_pts.size

    # 各特徴点の距離と角度: (手書き文字を辞書と比較)
    if sq_dist(i_pts[i_nop - 1], i_pts[i_nop - 2]) < limit_length
      i_k_end = i_nop - 2
    else
      i_k_end = i_nop - 1
    end
    m = 0
    i_k = 1
    while i_k < i_k_end
      i_pt = i_pts[i_k]
      i_me = i_met[i_k]
      d_k = m
      while d_k < d_nop
	d_pt = d_pts[d_k]
	if d_k < d_nop - 1
	  d_me = d_met[d_k]
	  if sq_dist(i_pt, d_pt) < limit_length &&
	      abs(i_me.angle - d_me.angle) < 3.14 / 2
	    m = d_k
	    break
	  else
	    # 各特徴点と線分との距離
	    r = d_me.a * i_pt.x + d_me.b * i_pt.y - d_me.e
	    if 0 <= r && r <= d_me.d * d_me.d &&
		d_me.a * i_pt.y - d_me.b * i_pt.x - d_me.c < limit_length * d_me.d &&
		abs(i_me.angle - d_me.angle) < 3.14 / 2
	      m = d_k
	      break
	    end
	  end
	else
	  if sq_dist(i_pt, d_pt) < limit_length
	    m = d_k
	    break
	  end
	end
	d_k += 1
      end
      if d_k >= d_nop
	return false
      end
      i_k += 1
    end
    return true
  end

  def match_dict_to_input(d_stroke, i_stroke, limit_length)
    d_pts = d_stroke.points
    d_met = d_stroke.metrics
    d_nop = d_pts.size
    i_pts = i_stroke.points
    i_met = i_stroke.metrics
    i_nop = i_pts.size

    # 各特徴点の距離と角度: (辞書を手書き文字と比較)
    if sq_dist(d_pts[d_nop - 1], d_pts[d_nop - 2]) < limit_length
      d_k_end = d_nop - 2
    else
      d_k_end = d_nop - 1
    end
    m = 0
    d_k = 1
    while d_k < d_k_end - 1
      d_pt = d_pts[d_k]
      d_me = d_met[d_k]
      i_k = m
      while i_k < i_nop
	i_pt = i_pts[i_k]
	if i_k < i_nop - 1
	  i_me = i_met[i_k]
	  if sq_dist(d_pt, i_pt) < limit_length &&
	      abs(i_me.angle - d_me.angle) < 3.14 / 2
	    m = i_k
	    break
	  else
	    # 各特徴点と線分との距離
	    r = i_me.a * d_pt.x + i_me.b * d_pt.y - i_me.e
	    if 0 <= r && r <= i_me.d * i_me.d &&
		i_me.a * d_pt.y - i_me.b * d_pt.x - i_me.c < limit_length * i_me.d &&
	        abs(i_me.angle - d_me.angle) < 3.14 / 2
	      m = i_k
	      break
	    end
	  end
	else
	  if sq_dist(d_pt, i_pt) < limit_length
	    m = i_k
	    break
	  end
	end
	i_k += 1
      end
      if i_k >= i_nop
	return false
      end
      d_k += 1
    end
    return true
  end

  def abs(x)
    return x >= 0 ? x : -x
  end

  def get_candidates(i_strokes, i_stroke_index, candidates)
    limit_length = (300 * 0.25) * (300 * 0.25)
    cache = []
    rtn_candidates = []
    for candidate in candidates
      char_index = candidate.char_index
      adapted = candidate.adapted.clone
      i_stroke = i_strokes[i_stroke_index]

      #
      # prefix memo: i_ = input, d_ = dict
      # abbrev memo: fp = first point, lp = last point
      #
      i_pts = i_stroke.points
      i_met = i_stroke.metrics
      i_nop = i_pts.size
      i_fp = i_pts[0]
      i_lp = i_pts[i_nop - 1]

      letter = @letters[char_index]
      for d_stroke_index in 0 .. letter.strokes.size - 1
        d_stroke = letter.strokes[d_stroke_index]
        if contains(adapted, d_stroke_index)
          next
        end

	if cache[char_index] == nil
	  cache[char_index] = []
	end
        if cache[char_index][d_stroke_index] == 1
          candidate.adapted.push(d_stroke_index)
          rtn_candidates.push(candidate.clone)
          break
        elsif cache[char_index][d_stroke_index] == -1
          next
        end
        cache[char_index][d_stroke_index] = -1

        d_pts = d_stroke.points
        d_met = d_stroke.metrics
        d_nop = d_pts.size
        d_fp = d_pts[0]
        d_lp = d_pts[d_nop - 1]

        # 始点・終点との距離, 特徴点の数
        if sq_dist(i_fp, d_fp) > limit_length ||
            sq_dist(i_lp, d_lp) > limit_length ||
            abs(d_nop - i_nop) > 3
          next
        end

        # 始線の角度 % 45 度 (PI/4) がしきい値
        i_dx = i_stroke.points[1].x - i_fp.x
        i_dy = i_stroke.points[1].y - i_fp.y
        d_dx = d_stroke.points[1].x - d_fp.x
        d_dy = d_stroke.points[1].y - d_fp.y
        if sq_len(i_dx, i_dy) > limit_length &&
            sq_len(d_dx, d_dy) > limit_length &&
            abs(d_met[0].angle - i_met[0].angle) > 3.14 / 4
          next
        end

        # 各特徴点の距離と角度: (手書き文字を辞書と比較)
	if !match_input_to_dict(i_stroke, d_stroke, limit_length)
	  next
	end

        # 各特徴点の距離と角度: (辞書を手書き文字と比較)
	if !match_dict_to_input(d_stroke, i_stroke, limit_length)
	  next
	end

	cache[char_index][d_stroke_index] = 1
	candidate.adapted.push(d_stroke_index)
	rtn_candidates.push(candidate.clone)
      end
    end
    return rtn_candidates
  end

  def match_stroke_num(strokes, char_index, adapted)
    i_stroke_num = strokes.size
    d_stroke_num = @letters[char_index].strokes.size
    if d_stroke_num - i_stroke_num >= 3
      pj = 100
      for j in adapted
	if j - pj >= 3
	  return false
	end
	pj = j
      end
    end
    return true
  end

  def get_matched_char_index_strokes_to_dict(strokes)
    matched = []
    candidates = match_char(strokes)
#    print "candidates=", candidates_to_s(candidates), "\n"
    pre_char = -1;
    for candidate in candidates
      char_index = candidate.char_index
      adapted = candidate.adapted
      if !match_stroke_num(strokes, char_index, adapted)
	pre_char = char_index
	next
      end
      if pre_char != char_index
	d_stroke_num = @letters[char_index].strokes.size
	if matched[d_stroke_num] == nil
	  matched[d_stroke_num] = []
	end
	matched[d_stroke_num].push(char_index)
      end
      pre_char = char_index
    end
    return matched
  end

  def print_matched_char_index(matched)
    i = 0
    for char_index_list in matched
      if char_index_list == nil
	next
      end
      for char_index in char_index_list
	if i > 0
	  print " "
	end
	print @letters[char_index].character
	i = i + 1
      end
    end
    print "\n"
  end

  def match_strokes_to_dict(strokes)
    matched = get_matched_char_index_strokes_to_dict(strokes)
    print_matched_char_index(matched)
  end
end

class StrokeLoader
  def load_strokes(file)
    strokes = []
    while ((line = file.gets) != nil)
      line.chomp!
      if line =~ /EOF/
        return nil
      elsif line =~ /^:[0-9]+/
        stroke_num = line[1 .. line.length].to_i
      else
        next
      end

      for i in 1 .. stroke_num
        line = file.gets
        line.chomp!
        point_num, rest = line.split(" ", 2)
        points = []
        rest.scan(/\(([0-9]+) ([0-9]+)\)\s?/) {|x, y|
          point = Point.new(x.to_i, y.to_i)
          points.push(point)
        }
        stroke = Stroke.new(points)
        strokes.push(stroke)
      end
      return strokes
#      letter = Letter.new("", strokes)
#      return letter
    end
  end
end

#p = Point.new
#p.x = 20
#p.y = 30
#print "p=" , p, "\n"
#
#q = Point.new(40, 50)
#print "q=" , q, "\n"
#
#points = [p, q]
#s = Stroke.new(points)
#print "s=" , s, "\n"
#
#strokes = [s]
#l = Letter.new("/", strokes)
#print "l=" , l, "\n"

if ARGV[0] == "-v"
  verbose = true
else
  verbose = false
end
dict = Dictionary.new("all.tdic")
#print "dict=" , dict.to_s, "\n"
while (strokes = StrokeLoader.new.load_strokes(STDIN)) != nil
  t1 = Time.now
  dict.match_strokes_to_dict(strokes)
  t2 = Time.now
  if verbose
    print "elapsed time(sec)=" + (t2 - t1).to_s + "\n"
  end
end

