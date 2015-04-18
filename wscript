# WAF buildscript
# :vim set ft=python

from waflib import Configure
from sys import path

# Project name
APPNAME = "libdy"
VERSION = "0.0.1"

# Paths
top = "."
out = "build"
path.insert(0, "local_waflib")

# Sub-Projects
projects = "libdy", "libdypp", "test", "pydy"


def init(ctx):
	# Automatically re-run configure when wscripts change
	#Configure.autoconfig=True
	pass

# build all projects
def options(ctx):
	ctx.add_option("-d", "--debug", action="store_true",
		help="Add debug information")
	ctx.add_option("-R", "--rpath", action="store_true",
		help="Add RPath information to built code")

	for project in projects:
		ctx.recurse(project)


def configure(ctx):
	ctx.check_waf_version(mini="1.8")
	
	ctx.start_msg("Build type")
	if ctx.options.debug:
		ctx.env.BUILD_TYPE = "Debug"
		ctx.env.append_unique("CFLAGS", ["-g"])
		ctx.env.append_unique("CXXFLAGS", ["-g"])
	else:
		ctx.env.BUILD_TYPE = "Release"
	ctx.end_msg(ctx.env.BUILD_TYPE)
	
	ctx.start_msg("Bake in rpath")
	if ctx.options.rpath:
		ctx.load("local_rpath")
		ctx.end_msg("yes")
	else:
		ctx.end_msg("no")

	for project in projects:
		ctx.recurse(project)


def build(ctx):
	for project in projects:
		ctx.recurse(project)
