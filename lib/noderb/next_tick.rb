module NodeRb

  class << self

    def next_tick params = nil, &block
      next_tick_schedule(block, params)
    end

    def next_tick_schedule block, params
      @next_tick ||= []
      @next_tick << [block, params]
      NodeRb.send(:next_tick_native)
    end

    def next_tick_execute
      blocks, @next_tick = @next_tick, []
      blocks.each do |block|
        block, params = block
        params ? block.call(params) : block.call
      end
    end

  end
end