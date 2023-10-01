project('fak', 'c')

cc_args = ['--opt-code-size']
inc_dirs = ['src', 'src/inc']

sources_common = [
    'src/main.c',
    'src/time.c',
    'src/bootloader.c',
]

sources_central = [
    'src/usb.c',
    'src/split_central.c',
    'src/keymap.c',
    'src/key_event_queue.c',
    'src/hold_tap.c',
    'src/tap_dance.c',
]

sources_peripheral = [
    'src/split_peripheral.c',
]

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

sides = ['central']
if get_option('split')
    sides += ['peripheral']
endif

foreach side : sides
    side_h = custom_target(side + '.h',
        input : ncl,
        output : side + '.h',
        capture : true,
        command : [jq, '-r', '.' + side + '.h', '@INPUT@'],
    )

    side_c = custom_target(side + '.c',
        input : ncl,
        output : side + '.c',
        capture : true,
        command : [jq, '-r', '.' + side + '.c', '@INPUT@'],
    )

    dir_base = meson.current_source_dir()
    cc_incs = ['--include', side_h.full_path()]

    foreach dir : inc_dirs
        cc_incs += '-I' + join_paths(dir_base, dir)
    endforeach

    compiler = generator(cc,
        output : '@BASENAME@.rel',
        arguments : cc_args + cc_incs + ['-c', '@INPUT@', '-o', '@OUTPUT@'],
        depends : [side_c, side_h],
    )

    sources = sources_common + [side_c]
    
    if side == 'central'
        sources += sources_central
    elif side == 'peripheral'
        sources += sources_peripheral
    endif

    rel = compiler.process(sources)

    ihx = custom_target(side + '.ihx',
        input : rel,
        output : side + '.ihx',
        command : [cc, cc_args, '-o', '@OUTPUT@', '@INPUT@'],
    )

    bin = custom_target(side + '.bin',
        input : ihx,
        output : side + '.bin',
        install : true,
        install_dir: 'firmware',
        command : [objcopy, '-I', 'ihex', '-O', 'binary', '@INPUT@', '@OUTPUT@'],
    )
endforeach
