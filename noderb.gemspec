require File.expand_path("../lib/noderb/version", __FILE__)
Gem::Specification.new do |s|
  s.name        = "noderb"
  s.version     = NodeRb::VERSION
  s.platform    = Gem::Platform::RUBY
  s.authors     = ["Marek Jelen"]
  s.email       = ["marek@jelen.biz"]
  s.homepage    = "http://github.com/marekjelen/noderb"
  s.summary     = "Port of the NodeJs library to Ruby"
  s.description = "Port of the NodeJs library to Ruby, featuring asynchronous IO operations of all kinds."

  s.required_rubygems_version = ">= 1.3.6"
  s.rubyforge_project         = "noderb"

  s.files        = Dir.glob("{bin,lib,ext}/**/*") + %w(LICENSE README.md)

  s.extensions << 'ext/noderb_extension/extconf.rb'

  s.require_path = 'lib'
end