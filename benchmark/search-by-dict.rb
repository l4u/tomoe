require "benchmark"

benchmark_dir = File.expand_path(File.dirname(__FILE__))
top_dir = File.expand_path(File.join(benchmark_dir, ".."))
test_dir = File.join(top_dir, "test")
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby", ".libs"))
$LOAD_PATH.unshift(File.join(top_dir, "ext", "ruby"))
$LOAD_PATH.unshift(File.join(test_dir))

require "tomoe-spec-utils"

n = 10
use_est = false
use_svn = true
tmp_dir = TomoeSpecUtils::Config.tmp_dir
Benchmark.bmbm do |x|
  TomoeSpecUtils::Config.dictionaries.sort.each do |dictionary|
    if use_est
      dict = Tomoe::Dict.new("est",
                             "name" => File.basename(dictionary),
                             "database_name" => dictionary.sub(/\.xml$/, ''),
                             "editable" => false)
    elsif use_svn
      repos = File.join(tmp_dir, "svn.repos", File.basename(dictionary))
      FileUtils.mkdir_p(repos)
      repos_url = "file://#{repos}"
      wc = File.join(tmp_dir, "svn.wc", File.basename(dictionary))
      dict_file = File.join(wc, "dict.xml")

      `svnadmin create #{repos.dump}`
      `svn co #{repos_url.dump} #{wc.dump}`
      FileUtils.cp(dictionary, dict_file)
      `svn add #{dict_file.dump}`
      `svn ci -m '' #{wc.dump}`

      xml_dict = Tomoe::Dict.new("xml",
                                 "filename" => dict_file,
                                 "editable" => true)
      dict = Tomoe::Dict.new("svn",
                             "dictionary" => xml_dict,
                             "working_copy" => wc)
    else
      dict = Tomoe::Dict.new("xml",
                             "filename" => dictionary,
                             "editable" => false)
    end

    x.report("#{File.basename(dictionary)}: all") do
      query = Tomoe::Query.new
      n.times {dict.search(query)}
    end

    x.report("#{File.basename(dictionary)}: >= 5") do
      query = Tomoe::Query.new
      query.min_n_strokes = 10
      n.times {dict.search(query)}
    end
  end
end

FileUtils.rm_rf(tmp_dir)
