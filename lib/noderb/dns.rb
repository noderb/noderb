module NodeRb

  class << self

    def resolve hostname, &block
      NodeRb.resolve_native(hostname, block)
    end

  end

end