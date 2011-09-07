module NodeRb

	class FileSystem

		class << self

			def unlink path, &block
				fs_native(0, path, nil, block)
			end

			def mkdir path, mode, &block
				fs_native(1, path, mode, block)
			end

			def rmdir path, &block
				fs_native(2, path, nil, block)
			end

			def readdir path, flags = 2, &block
				parser = proc do |data|
					data ? block.call(data.split("\0")) : block.call(data)
				end
				fs_native(3, path, flags, parser)
			end

			def stat path, &block
				fs_native(4, path, nil, block)
			end

			def rename path, new_path, &block
				fs_native(5, path, new_path, block)
			end

			def chmod path, mode, &block
				fs_native(6, path, mode, block)
			end

			def utime path, atime, mtime, &block
				fs_native(7, path, [atime, mtime], block)
			end

			def lstat path, &block
				fs_native(8, path, nil, block)
			end

			def link path, new_path, &block
				fs_native(9, path, new_path, block)
			end

			def symlink path, new_path, flags, &block
				fs_native(10, path, [new_path, flags], block)
			end

			def readlink path, &block
				# ToDo: libuv not implemented yet
				block.call(nil)
				#fs_native(11, path, nil, block)
			end

			def chown path, uid, gid, &block
				fs_native(12, path, [uid, gid], block)
			end

		end

	end

	class Stat
		
	end

end