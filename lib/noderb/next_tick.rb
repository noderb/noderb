module NodeRb
  class << self
    
    def next_tick &block
      ( @next_tick ||= [] ) << block
      NodeRb.native_next_tick
    end
    
    def next_tick_execute
      blocks, @next_tick = @next_tick, []
      blocks.each do |block|
        block.call
      end
    end
    
  end
end