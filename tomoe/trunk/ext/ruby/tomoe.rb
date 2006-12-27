require 'glib2'
require 'tomoe.so'

module Tomoe
  LOG_DOMAIN = "Tomoe"
end

GLib::Log.set_log_domain(Tomoe::LOG_DOMAIN)
