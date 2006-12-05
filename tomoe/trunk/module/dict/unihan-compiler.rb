#!/usr/bin/env ruby

require 'uconv'
require 'suikyo/suikyo'

unihan_txt = ARGV.shift

DO_NOT_EDIT_HEADER = <<-EOH
/*
  DO NOT EDIT!
  THIS FILE IS GENERATED FROM Unihan.txt:
    ftp://ftp.unicode.org/Public/UNIDATA/Unihan.zip
*/
EOH

@romaji_to_hiragana = Suikyo.new("romaji-kana")
@hiragana_to_katakana = Suikyo.new("hiragana-katakana")

def euc_to_utf8(euc)
  Uconv.euctou8(euc)
end

def romaji_to_hiragana(romaji)
  euc_to_utf8(@romaji_to_hiragana.convert("#{romaji} "))
end

def romaji_to_katakana(romaji)
  hiragana = @romaji_to_hiragana.convert("#{romaji} ")
  euc_to_utf8(@hiragana_to_katakana.convert("#{hiragana} "))
end

def ucs4_to_utf8(ucs4)
  Uconv.u4tou8([Integer("0x#{ucs4}")].pack("I*"))
end

def parse_unihan_txt(unihan_txt)
  cache = "#{unihan_txt}.cache"
  if File.exists?(cache) and (File.mtime(cache) > File.mtime(unihan_txt))
    begin
      return Marshal.load(File.read(cache))
    rescue ArgumentError
    end
  end

  infos = {}
  File.open(unihan_txt).each do |line|
    case line
    when /^#/ #
      next
    when /^U\+([\da-fA-F]+)\s+([a-zA-Z_]+)\s*(.*)\s*$/u
      ucs4 = $1
      key = $2
      value = $3

      infos[ucs4] ||= {}
      infos[ucs4][key] = value
    else
      STDERR.puts "Unknown line: #{line}"
    end
  end

  result = infos.collect do |ucs4, info|
    [ucs4, info]
  end.sort_by do |ucs4, info|
    ucs4
  end

  File.open(cache, "wb") {|f| f.print(Marshal.dump(result))}
  result
end

def generate_header(infos)
  prefix = "tomoe_unihan_"

  puts <<-EOH
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#{DO_NOT_EDIT_HEADER}

#include "tomoe-unihan.h"

#include <tomoe-char.h>

typedef struct _TomoeUnihanReading TomoeUnihanReading;
typedef struct _TomoeUnihanInfo TomoeUnihanInfo;

struct _TomoeUnihanReading {
    TomoeReadingType  type;
    gchar            *reading;
};

struct _TomoeUnihanInfo {
    gchar               *utf8;
    gint                 n_strokes;
    gchar              **variants;
    TomoeUnihanReading  *readings;
};

EOH

  infos.each_with_index do |(ucs4, info), i|
    variants = info["kCompatibilityVariant"]
    if variants
      info["have_variants"] = true
      puts("static gchar *#{prefix}variant_#{ucs4}[] = {")
      variants.split.each do |variant|
        utf8_variant = ucs4_to_utf8(variant.sub(/^U\+2?/, ''))
        puts("    \"#{utf8_variant}\",")
      end
      puts("};");
    end

    readings = []
    kuns = info["kJapaneseKun"]
    ons = info["kJapaneseOn"]
    if kuns
      readings.concat(kuns.split.collect do |x|
                        ["TOMOE_READING_JA_KUN", romaji_to_hiragana(x)]
                      end)
    end
    if ons
      readings.concat(ons.split.collect do |x|
                        ["TOMOE_READING_JA_ON", romaji_to_katakana(x)]
                      end)
    end

    unless readings.empty?
      info["have_readings"] = true
      puts("static TomoeUnihanReading #{prefix}reading_#{ucs4}[] = {")
      readings.each do |type, reading|
        puts("    {#{type}, \"#{reading}\"},")
      end
      puts("};")
    end
  end

  puts("static TomoeUnihanInfo #{prefix}infos[] = {")
  infos.each_with_index do |(ucs4, info), i|
    info["utf8"] = utf8 = Uconv.u4tou8([Integer("0x#{ucs4}")].pack("I*"))
    n_strokes = info["kTotalStrokes"] || -1
    variants = info["have_variants"] ? "#{prefix}variant_#{ucs4}" : "NULL"
    readings = info["have_readings"] ? "#{prefix}reading_#{ucs4}" : "NULL"

    puts("    {\"#{utf8}\", #{n_strokes}, #{variants}, #{readings}},")
  end
  puts("};")
end

infos = parse_unihan_txt(unihan_txt)
generate_header(infos)
