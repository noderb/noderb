require "mkmf"

LIBUV = "75c109055d330bdd6177"

`cd libuv; git fetch; git reset --hard #{LIBUV}; make; cd ..; cp libuv/uv.a libuv.a`

dir_config("uv", File.expand_path("../libuv/include", __FILE__), File.expand_path("../libuv", __FILE__))

have_library("uv")

create_makefile("noderb_extension")