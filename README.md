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

Connects to specific hostname/IP address and port using TCP.

	class Handler
	
		include NodeRb::Connection
		
		def on_connect
			# Do something
			write_data("Some data")
		end
		
		def on_data data
			# Do something
			close_connection
		end
		
		def on_close
			# Do something
		end
		
	end
	
	NodeRb.start_client("host", port, Handler.new)

### Starting server

Starts new server on specific hostname/IP address and port using TCP.

	class Handler

		include NodeRb::Connection

		def on_connect
			# Do something
			write_data("Some data")
		end

		def on_data data
			# Do something
			close_connection
		end

		def on_close
			# Do something
		end

	end
		
	NodeRb.start_server("host", port, Handler)

### Starting child process

Starts new process in specific directory and with specific environment.

	class Handler
	
		include NodeRb::Process
		
		def on_start
			# Do something
			write_data("Some data")
		end
		
		def on_stdout data
			# Do something
			kill_process
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

### Resolving hostnames

Resolves hostname to IP address.

	NodeRb.resolve("hostname") do |ip|
		# Do something with the IP address
	end
	
### Scheduling events

Runs specified code in the future of *timeout* seconds.

	NodeRb.once(timeout) do
		# Do something
	end
	
Schedules new repeated event that will be called every *timeout* seconds.

	class Handler

		include NodeRb::Timer
	
		def call
			# Do something
			stop_timer
		end
	
	end
	
	NodeRb.repeat(timeout, Handler.new)

## ToDo

* File IO
* UDP support
* Better documentation
* Tests
* ensure Windows support
* lots, lots more

## Changelog

### 0.0.4

* Hostname resolution
* Timers support
* Huge code refactorings

## Compatibility

* Full
** Ruby 1.8
** Ruby 1.9
* Preliminary
** Rubinius

## Dependencies

No external dependencies except compiler for native extension.

## License

MIT