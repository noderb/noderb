module NodeRb

  class << self
    
    def next_tick &block
      ( @next_tick ||= [] ) << block
      NodeRb.send(:next_tick_native)
    end
    
    def next_tick_execute
      blocks, @next_tick = @next_tick, []
      blocks.each do |block|
        block.call
      end
    end
    
  end
end