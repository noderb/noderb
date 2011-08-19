require "mkmf"

`cd libuv; make; cd ..; cp libuv/uv.a libuv.a`

dir_config("uv", File.expand_path("../libuv/include", __FILE__), File.expand_path("../libuv", __FILE__))

have_library("uv")

create_makefile("noderb_extension")