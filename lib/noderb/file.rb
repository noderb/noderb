module NodeRb

	module File

		# Operations

		def open file, flags = "r", mode = 600
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

		def read size = 1024, offset = 0
			operation_native(1, [size, offset])
		end

		def write data
			operation_native(2, data)
		end

		def stat
			operation_native(3, nil)
		end

		def sync
			operation_native(4, nil)
		end

		def datasync
			operation_native(5, nil)
		end

		def truncate offset = 0
			operation_native(6, offset)
		end

		def sendfile output, offset = 0, length = 1024
			operation_native(7, [output, offset, length])
		end

		def utime atime, mtime
			operation_native(8, [atime, mtime])
		end

		def chmod mode
			operation_native(9, mode)
		end

		def chown uid, gid
			operation_native(10, [uid, gid])
		end

		def close
			operation_native(11, nil)
		end

		# Callbacks

		def on_open

		end

		def on_read data

		end

		def on_write

		end

		def on_stat
			
		end

		def on_sync
			
		end

		def on_datasync
			
		end

		def on_truncate
			
		end

		def on_sendfile
			
		end

		def on_utime

		end

		def on_chmod
			
		end

		def on_chown
			
		end

		def on_close

		end

		def on_error
			
		end

	end

end