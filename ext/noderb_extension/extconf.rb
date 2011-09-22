require "mkmf"

cflags = " -shared -fPIC "

if $solaris
  cflags = " -G -fPIC "
  # From EventMachine
  CONFIG['LDSHARED'] = "$(CXX) -G -fPIC"
  if CONFIG['CC'] == 'cc'
    cflags = "-g -O2 -fPIC"
    CONFIG['CCDLFLAGS'] = "-fPIC"
  end
end

$CFLAGS = CONFIG['CFLAGS'] = cflags

`cd libuv; CPPFLAGS="#{cflags}" make; cd ..; cp libuv/uv.a libuv.a` # libuv does not respond to CFLAGS anymore

dir_config("uv", File.expand_path("../libuv/include", __FILE__), File.expand_path("../libuv", __FILE__))

have_library("uv")

create_makefile("noderb_extension")