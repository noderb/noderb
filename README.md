# NodeRb

NodeRb = NodeJs - JavaScript + Ruby

## Status

It could not be more prototype than it is.

## Instalation

```
gem install noderb
```

## Usage

### Starting loop

```ruby
require "noderb"

NodeRb.start do
  # Do something
end

# or just

NodeRb.start
```

### Running code on next tick

```ruby
NodeRb.next_tick do
  # Do something
end
```

### Connecting to server

Connects to specific hostname/IP address and port using TCP.

```ruby
class Handler

  include NodeRb::Connection

  def on_connection_open
    # Do something
    connection_write("Some data")
  end

  def on_connection_read(data)
    # Do something
    connection_close
  end

  def on_connection_close
    # Do something
  end

end

NodeRb.start_client("host", port, Handler.new)
```

### Starting server

Starts new server on specific hostname/IP address and port using TCP.

```ruby
class Handler

  include NodeRb::Connection

  def on_connection_open
    # Do something
    connection_write("Some data")
  end

  def on_connection_read(data)
    # Do something
    connection_close
  end

  def on_connection_close
    # Do something
  end

end

NodeRb.start_server("host", port, Handler)
```

### Filesystem access

Methods implementing asynchronous access to filesystem.

```ruby
NodeRb::FileSystem.unlink(path, &block)
NodeRb::FileSystem.mkdir(path, mode, &block)
NodeRb::FileSystem.rmdir(path, &block)
NodeRb::FileSystem.readdir(path, flags, &block)
NodeRb::FileSystem.stat(path, &block)
NodeRb::FileSystem.rename(path, new_path, &block)
NodeRb::FileSystem.chmod(path, mode, &block)
NodeRb::FileSystem.utime(path, access_time, modification_time, &block)
NodeRb::FileSystem.lstat(path, &block)
NodeRb::FileSystem.link(path, new_path, &block)
NodeRb::FileSystem.symlink(path, new_path, flags, &block)
NodeRb::FileSystem.readlink(path, &block)
NodeRb::FileSystem.chown(path, uid, gid, &block)
```

All methods take block that will be called when operation is finished.

### File access

```ruby
class Handler

  include NodeRb::File

  def do_some_work
    file_open(path, flags, mode)
    file_read(length, offset)
    file_write(data)
    file_stat
    file_sync
    file_datasync
    file_truncate(offset)
    file_sendfile(output, offset, length)
    file_utime(access_time, modification_time)
    file_chmod(mode)
    file_chown(uid, gid)
    file_close
  end

  # Every operations when finishes calls appropriate callback or on_file_error when error occurs

  def on_file_open
    # Do some work
  end

  def on_file_read(data)
    # Do some work
  end

  def on_file_write
    # Do some work
  end

  def on_file_stat
    # Do some work
  end

  def on_file_sync
    # Do some work
  end

  def on_file_datasync
    # Do some work
  end

  def on_file_truncate
    # Do some work
  end

  def on_file_sendfile
    # Do some work
  end

  def on_file_utime
    # Do some work
  end

  def on_file_chmod
    # Do some work
  end

  def on_file_chown
    # Do some work
  end

  def on_file_close
    # Do some work
  end

  def on_file_error
    # Do some work
  end

end
```

### Starting child process

Starts new process in specific directory and with specific environment.

```ruby
class Handler

  include NodeRb::Process

  def on_process_open
    # Do something
    process_write("Some data")
  end

  def on_process_stdout(data)
    # Do something
    process_close
  end

  def on_process_stderr(data)
    # Do something
  end

  def on_process_close(status, signal)
    # Do something
  end
end

args = ["argument", "list"]
env = ["VAR1=value", "VAR2=value"]

NodeRb.start_process("executable", args, env, "working_directory", Handler.new)
```

### Resolving hostnames

Resolves hostname to IP address.

```ruby
NodeRb.resolve("hostname") do |ip|
  # Do something with the IP address
end
```

### Scheduling events

Runs specified code in the future of *timeout* seconds.

```ruby
NodeRb.once(timeout) do
  # Do something
end
```

Schedules new repeated event that will be called every *timeout* seconds.

```ruby
class Handler

  include NodeRb::Timer

  def call
    # Do something
    stop_timer
  end

end

NodeRb.repeat(timeout, Handler.new)
```

## ToDo

* UDP support
* Better documentation
* Tests
* ensure Windows support
* lots, lots more

## Changelog

### 0.0.6

* FileSystem support

### 0.0.4

* Hostname resolution
* Timers support
* Huge code refactorings

## Compatibility

* Ruby 1.9
* Ruby 1.8
* Rubinius

* Linux (Fedora, Ubuntu)
* MacOS X
* Solaris (Illumos)

## Dependencies

No external dependencies except compiler for native extension.

## License

MIT