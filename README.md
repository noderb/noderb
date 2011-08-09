# NodeRb

NodeRb = NodeJs - JavaScript + Ruby

## Status

It could not be more prototype than it is.

## Instalation

    gem install noderb

## Usage

### Starting loop

	require "noderb"
	
	NodeRb.start do
		# Do something
	end

### Running code on next tick

	NodeRb.next_tick do
		# Do something
	end
	
### Connecting to server

	class Handler
	
		include NodeRb::Connection
		
		def on_connect
			# Do something
			# write("Some data")
		end
		
		def on_data data
			# Do something
		end
		
		def on_close
			# Do something
		end
		
	end
	
	NodeRb.start_client("IP address", port, Handler.new)

### Starting server

	class Handler

		include NodeRb::Connection

		def on_connect
			# Do something
			# write("Some data")
		end

		def on_data data
			# Do something
		end

		def on_close
			# Do something
		end

	end
		
	NodeRb.start_server("IP address", port, Handler)

### Starting child process

	class Handler
	
		include NodeRb::Process
		
		def on_start
			# Do something
			# write("Some data")
		end
		
		def on_stdout data
			# Do something
		end
		
		def on_stderr data
			# Do something
		end
		
		def on_exit status, signal
			# Do something
		end
	end
	
	args = ["argument", "list"]
	env = ["VAR1=value", "VAR2=value"]
	
	NodeRb.start_process("executable", args, env, "working_directory", Handler.new)
	
## ToDo

* DNS resolving
* File IO
* ensure Windows support

* lots, lots more

## Dependencies

No external dependencies except compiler for native extension.

## License

MIT