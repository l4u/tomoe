require 'rubygems'
require 'active_record'

require 'yaml'

base = File.expand_path(File.dirname(__FILE__))
config_file = File.join(base, "config.yml")

config = YAML.load(File.read(config_file))
ActiveRecord::Base.establish_connection(config)

logger = Logger.new($stdout)
logger.level = Logger::DEBUG
ActiveRecord::Base.logger = logger
