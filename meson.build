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
]

sources_peripheral = [
    'src/split_peripheral.c',
]

###

cc = find_program('sdcc', required : true)
nickel = find_program('nickel', required : true)
python = find_program('python', required : true)
wchisp = find_program('wchisp', required : false, disabler : true)

sides = ['central']
if get_option('split')
    sides += ['peripheral']
endif

foreach side : sides
    side_h = custom_target(side + '.h',
        input : 'fak.py',
        output : side + '.h',
        capture : true,
        command : [python, '@INPUT@', 'query_ncl', side + '.h'],
        depend_files : ['.main.ncl.json'],
    )

    side_c = custom_target(side + '.c',
        input : 'fak.py',
        output : side + '.c',
        capture : true,
        command : [python, '@INPUT@', 'query_ncl', side + '.c'],
        depend_files : ['.main.ncl.json'],
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

        extra_sources = get_option('extra_sources').strip()
        if extra_sources != ''
            sources += extra_sources.split(',')
        endif
    elif side == 'peripheral'
        sources += sources_peripheral
    endif

    rel = compiler.process(sources)

    ihx = custom_target(side + '.ihx',
        input : rel,
        output : side + '.ihx',
        install : true,
        install_dir : 'firmware',
        command : [cc, cc_args, '-o', '@OUTPUT@', '@INPUT@'],
    )

    flash = run_target('flash_' + side,
        command : [wchisp, 'flash', ihx.full_path()],
        depends : ihx,
    )
endforeach

wchisp_info = run_target('wchisp_info',
    command : [wchisp, 'info'],
)
