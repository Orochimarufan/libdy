# WAF buildscript
# :vim set ft=python

from waflib import Configure
from sys import path, version_info
from subprocess import check_output

# Project name
APPNAME = "libdy"
VERSION = "0.0.1"

# Paths
top = "."
out = "build"
path.insert(0, "local_waflib")

# Sub-Projects
projects = "libdy", "libdy++", "test", "pydy"


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
    ctx.add_option("-D", "--no-docs", action="store_true",
        help="Don't build the Doxygen documentation")

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

    if ctx.options.no_docs:
        ctx.env.NO_DOXYGEN="1"

    ctx.find_program("git", var="GIT", mandatory=False)
    
    if ctx.env.GIT:
        ctx.start_msg("Git revision")
        ctx.end_msg(get_git_rev(ctx.env.GIT))

    for project in projects:
        ctx.recurse(project)


def build(ctx):
    ctx.git_rev = get_git_rev(ctx.env.GIT)

    for project in projects:
        ctx.recurse(project)


def get_git_rev(git):
    if not git:
        print("XX")
        return "git-unknown"

    try:
        rev = check_output([git[0], "show-ref", "-s", "--abbrev", "HEAD"]).strip()
    except:
        return "git-unknown"
    
    if version_info > (3,):
        return rev.decode()
    return rev
