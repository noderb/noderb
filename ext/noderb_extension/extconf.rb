require "mkmf"

cflags = "-shared -fPIC"
ldflags = ""

case RUBY_PLATFORM
  when /solaris/
    cflags = " -G -fPIC "
    CONFIG['LDSHARED'] = "$(CXX) -G -fPIC"
    if CONFIG['CC'] == 'cc'
      cflags = "-g -O2 -fPIC"
      CONFIG['CCDLFLAGS'] = "-fPIC"
    end
  when /darwin/
    ldflags = "-framework CoreServices"
end

$CFLAGS = CONFIG['CFLAGS'] = " #{cflags} "
$LDFLAGS = CONFIG['LDFLAGS'] = " #{ldflags} "

`cd libuv; CFLAGS="#{cflags}" make; cd ..; cp libuv/uv.a libuv.a`

dir_config("uv", File.expand_path("../libuv/include", __FILE__), File.expand_path("../libuv", __FILE__))

have_library("uv")

create_makefile("noderb_extension")