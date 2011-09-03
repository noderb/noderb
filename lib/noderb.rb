# Native extension
require "noderb_extension"
# Tools
require "noderb/version"
require "noderb/next_tick"
require "noderb/timers"
require "noderb/defer"
# System
require "noderb/tcp"
require "noderb/dns"
# FS
require "noderb/fs"
# Helpers
require "noderb/connection"
require "noderb/process"

module NodeRb

  class << self
    
    @instances = []

    def register_instance instance
      @instances << instance
    end

    def unregister_instance instance
      @instances.delete(instance)
    end

  end

end