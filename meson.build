project('fak', 'c')

fs = import('fs')

cc_args = ['--opt-code-size']
inc_dirs = ['src', 'src/inc']

sources_common = [
    'src/main.c',
    'src/time.c',
    'src/bootloader.c',
]

sources_c = [
    'src/usb.c',
    'src/split_central.c',
    'src/keymap.c',
    'src/key_event_queue.c',
    # 'src/hold_tap.c',
    # 'src/tap_dance.c',
]

sources_p = [
    'src/split_peripheral.c',
]

sources = sources_common + sources_p

###

cc = find_program('sdcc', required : true)
objcopy = find_program('objcopy', required : true)
nickel = find_program('nickel', required : true)
jq = find_program('jq', required : true)

ncl = custom_target('ncl',
    input : 'ncl/fak/main.ncl',
    output : '@PLAINNAME@.json',
    capture : true,
    command : [nickel, 'export', '--format', 'json', '-f', '@INPUT@'],
    build_always_stale : true,
)

user_c = custom_target('user.c',
    input : ncl,
    output : 'user.c',
    capture : true,
    command : [jq, '-r', '.user.c', '@INPUT@'],
)

user_h = custom_target('user.h',
    input : ncl,
    output : 'user.h',
    capture : true,
    command : [jq, '-r', '.user.h', '@INPUT@'],
)

sources += [user_c]

###

dir_base = meson.current_source_dir()
cc_incs = ['--include', user_h.full_path()]

foreach dir : inc_dirs
    cc_incs += '-I' + join_paths(dir_base, dir)
endforeach

compiler = generator(cc,
    output : '@BASENAME@.rel',
    arguments : cc_args + cc_incs + ['-c', '@INPUT@'] + ['-o', '@OUTPUT@'],
    depends : [user_c, user_h],
)

rel = compiler.process(sources)

ihx = custom_target('main.ihx',
    input : rel,
    output : 'main.ihx',
    command : [cc, cc_args, '-o', '@OUTPUT@', '@INPUT@'],
)

bin = custom_target('main.bin',
    input : ihx,
    output : 'main.bin',
    install : true,
    install_dir: 'firmware',
    command : [objcopy, '-I', 'ihex', '-O', 'binary', '@INPUT@', '@OUTPUT@'],
)
