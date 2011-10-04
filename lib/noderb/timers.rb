module NodeRb

  class << self

    def once timeout, &block
      NodeRb.once_native(timeout * 1000, 0, block)
    end

    def repeat repeat, handler, timeout = nil
      timeout ||= repeat
      NodeRb.once_native(timeout * 1000, repeat * 1000, handler)
    end

  end

  module Timer

    def call

    end

  end

end