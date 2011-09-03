module NodeRb

  module File

    def open file, mode = 600, flags = "rw"
      flags = case flags
               when "rw"
                 0
               when "r"
                 1
               when "w"
                 2
             end
      open_native(file, flags, mode)
    end

    def write data
      write_native(data)
    end

    def read size = 1024, from = 0
      read_native(size, from)
    end

    def close
      close_native
    end

    def on_open

    end

    def on_read data

    end

    def on_write
      
    end

    def on_close

    end

  end

end