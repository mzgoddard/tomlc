#!./waf

import os
import subprocess

def options(ctx):
    ctx.load('compiler_c')

import waflib.Logs
def configure(ctx):
    ctx.load('compiler_c')

    ctx.env.append_value( 'CFLAGS', '-g' )
    ctx.env.append_value( 'CFLAGS', '-O2' )
    ctx.env.append_value( 'CFLAGS', '-std=c99' )

    ctx.start_msg( 'init submodules' )
    gitStatus = ctx.exec_command( 'git submodule init && git submodule update' )
    if gitStatus == 0:
        ctx.end_msg( 'ok', 'GREEN' )
    else:
        ctx.fatal( 'fail' )

    ctx.start_msg( 'build libtap dependency' )
    libtapStatus = ctx.exec_command( 'cd vendor/libtap && make' )
    if libtapStatus == 0:
        ctx.end_msg( 'ok', 'GREEN' )
    else:
        ctx.fatal( 'fail' )

from waflib.TaskGen import extension

from waflib.Task import Task

class re2c(Task):
    run_str = 're2c ${SRC} > ${TGT}'
    color = 'PINK'

@extension('re2c')
def process_re2c( self, node ):
    task = self.create_task( 're2c', node, node.change_ext( '.c' ) )
    self.source.extend( task.outputs )

def build(bld):
    bld.install_files( '${PREFIX}/include', 'toml.h' )

    source = bld.path.ant_glob('toml.c toml-lemon.c toml-re2c.re2c')
    d = {
        'source': source,
        'includes': '.',
        'target': 'toml',
        'install_path': '${PREFIX}/lib'
        # 'lib': 'antlr3c'
    }
    bld.stlib( **d )
    bld.shlib( **d )

    binSource = list( source )
    binSource.extend( bld.path.ant_glob('main.c') )
    bld.program(
        source=binSource,
        includes='.',
        target='toml',
        install_path='${PREFIX}/bin'
    )

    bld.program(
        source=bld.path.ant_glob('test.c'),
        includes='. ../vendor/libtap',
        target='test',
        libpath='../vendor/libtap',
        ldflags='-static',
        lib='tap',
        use='toml',
        install_path=None
    )

import waflib.Scripting, waflib.Context

def test(ctx):
    waflib.Scripting.run_command( 'build' )
    ctx.to_log( '\n' )
    ctx.cmd_and_log(
        'build/test\n',
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        quiet=waflib.Context.BOTH
    )
