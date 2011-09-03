module NodeRb

  class BackWorker < Thread

    def initialize block
      super do
        block.call
      end
    end

  end

  class << self

    def defer work = nil, callback = nil, &block
      work = block if not work and block
      callback = block if not callback and block
      work ||= proc { }
      callback ||= proc { |result| }
      NodeRb.defer_native(BackWorker.new(work), callback)
    end

  end

end