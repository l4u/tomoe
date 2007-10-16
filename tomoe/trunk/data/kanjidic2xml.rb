#!/usr/bin/env ruby

require 'nkf'
require 'erb'
require 'ostruct'
require 'English'

def parse_line(line)
  info = OpenStruct.new
  info.utf8, jis_code, rest = line.split(/ /u, 3)
  ku, ten = jis_code.scan(/(..)/u).flatten.collect {|x| x.hex - 0x20}
  info.n_strokes = []
  info.meta = {:jis208 => "#{ku}-#{ten}"}
  info.readings = {:on => [], :kun => []}
  meanings = []
  nanori = radical = false
  until rest.empty?
    case rest
    when /\A\{/u
      _, meaning, rest = rest.split(/[{}]/u, 3)
      meanings << meaning
    when /\A(MN|MP|[A-Z])/u
      key = $1
      value, rest = $POSTMATCH.split(/ /u, 2)
      case key
      when "U"
        info.meta[:ucs] = value
      when "G"
        info.meta[:jouyou] = value
      when "S"
        info.n_strokes << value
      when "T"
        case value
        when "1"
          nanori = true
        when "2"
          radical = true
        end
      end
    else
      reading, rest = rest.split(/ /u, 2)
      unless nanori
        if /-/ =~ reading or reading == NKF.nkf("-Ww --hiragana", reading)
          info.readings[:kun] << reading
        else
          info.readings[:on] << reading
        end
      end
    end
    rest = rest.lstrip
  end
  info.meta[:meaning] = meanings.join(", ") unless meanings.empty?
  info
end

def output_character(info)
  puts <<-EOC
  <character>
    <utf8>#{info.utf8}</utf8>
EOC
  info.n_strokes.each do |n|
    puts "    <number-of-strokes>#{n}</number-of-strokes>"
  end
puts <<-EOC
    <readings>
EOC
  [:on, :kun].each do |key|
    info.readings[key].each do |value|
      puts "      <reading type=\"ja_#{key}\">#{value}</reading>"
    end
  end
  puts <<-EOC
    </readings>
    <meta>
EOC
  [:jis208, :ucs, :jouyou, :meaning].each do |key|
    value = info.meta[key]
    puts "      <#{key}>#{ERB::Util.h(value)}</#{key}>" if value
  end
  puts <<-EOC
    </meta>
  </character>
EOC
end


puts <<-EOH
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE dictionary SYSTEM "dict.dtd">
<dictionary name="Jim Breen's KanjiDic">
EOH

ARGF.each_line do |line|
  next if /^#/ =~ line
  output_character(parse_line(NKF.nkf("-Ew", line.rstrip)))
end

puts "</dictionary>"
