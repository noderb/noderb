module NodeRb

  module File

    # Operations

    def file_open(file, flags = "r", mode = 600)
      flags = case flags
                when "r"
                  1
                when "w"
                  2
                else
                  0
              end
      operation_native(0, [file, flags, mode])
    end

    def file_read(size = 1024, offset = 0)
      operation_native(1, [size, offset])
    end

    def file_write(data)
      operation_native(2, data)
    end

    def file_stat
      operation_native(3, nil)
    end

    def file_sync
      operation_native(4, nil)
    end

    def file_datasync
      operation_native(5, nil)
    end

    def file_truncate(offset = 0)
      operation_native(6, offset)
    end

    def file_sendfile(output, offset = 0, length = 1024)
      operation_native(7, [output, offset, length])
    end

    def file_utime(atime, mtime)
      operation_native(8, [atime, mtime])
    end

    def file_chmod(mode)
      operation_native(9, mode)
    end

    def file_chown(uid, gid)
      operation_native(10, [uid, gid])
    end

    def file_close
      operation_native(11, nil)
    end

    # Callbacks

    def on_file_open

    end

    def on_file_read(data)

    end

    def on_file_write

    end

    def on_file_stat

    end

    def on_file_sync

    end

    def on_file_datasync

    end

    def on_file_truncate

    end

    def on_file_sendfile

    end

    def on_file_utime

    end

    def on_file_chmod

    end

    def on_file_chown

    end

    def on_file_close

    end

    def on_file_error

    end

  end

end