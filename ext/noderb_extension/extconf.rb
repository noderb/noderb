require "mkmf"

cflags = " -shared "

if $solaris
  cflags = " -G -fPIC "
  # From EventMachine
  CONFIG['LDSHARED'] = "$(CXX) -G -fPIC"
  if CONFIG['CC'] == 'cc'
    $CFLAGS = CONFIG['CFLAGS'] = "-g -O2 -fPIC"
    CONFIG['CCDLFLAGS'] = "-fPIC"
  end
end

`cd libuv; CFLAGS="#{cflags}" make; cd ..; cp libuv/uv.a libuv.a`

dir_config("uv", File.expand_path("../libuv/include", __FILE__), File.expand_path("../libuv", __FILE__))

have_library("uv")

create_makefile("noderb_extension")