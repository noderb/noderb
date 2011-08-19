module NodeRb

  class << self

    def start_server address, port, clazz
      if /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/.match(address)
        NodeRb.start_server_native(address, port, clazz)
      else
        NodeRb.resolve(address) do |ip|
          NodeRb.start_server_native(ip, port, clazz)
        end
      end
    end

    def start_client address, port, clazz
      if /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/.match(address)
        NodeRb.start_client_native(address, port, clazz)
      else
        NodeRb.resolve(address) do |ip|
          NodeRb.start_client_native(ip, port, clazz)
        end
      end
    end

  end
  
end