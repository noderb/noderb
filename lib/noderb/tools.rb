module NodeRb

  def self.kill
    NodeRb.next_tick do
      puts "Exiting"
      Kernel.exit(0)
    end
  end

end