#!/usr/bin/env python
import subprocess
import json
import glob
import hashlib
import os
import sys
import time
import shutil

os.chdir(sys.path[0])

EVAL_PATH = '.eval.json'
BUILD_DIR = 'build'
SUBCOMMAND = sys.argv[1]
HASH_MANAGED = '!managed'


def compute_hash_sig():
    h = hashlib.sha256()

    for filepath in glob.glob('ncl/**/*.ncl', recursive=True):
        with open(filepath, 'r') as f:
            h.update(f.read().encode('utf-8'))

    return h.hexdigest()


def save_evaluation():
    completed_proc = subprocess.run(
        ['nickel', 'export', '-Incl', 'tests/eval.ncl'],
        capture_output=True,
        text=True,
    )

    if completed_proc.returncode != 0:
        print(completed_proc.stderr)
        sys.exit(1)
    
    raw_result = completed_proc.stdout
    result = json.loads(raw_result)
    result['__hash__'] = compute_hash_sig()

    with open(EVAL_PATH, 'w') as f:
        f.write(json.dumps(result, indent=2))
    
    return result


def load_evaluation():
    if not os.path.isfile(EVAL_PATH):
        return None
    
    with open(EVAL_PATH, 'r') as f:
        result = json.loads(f.read())
        rhash = result['__hash__']

        if rhash == HASH_MANAGED or rhash == compute_hash_sig():
            return result
    
    return None


def evaluate_ncl():
    result = load_evaluation()

    if result is None:
        result = save_evaluation()
    
    return result


def subcmd_query_ncl():
    result = evaluate_ncl()
    selector = sys.argv[2]

    for key in selector.split('.'):
        result = result[key]
    
    print(result, end='')


def meson_configure():
    if not os.path.isdir(BUILD_DIR):
        subprocess.run(['meson', 'setup', BUILD_DIR], check=True)

    print("Evaluating Nickel files...")
    result = evaluate_ncl()

    if result['__hash__'] == HASH_MANAGED:
        print("Info: This is a managed evaluation.")

    for key, value in result['meson_options'].items():
        subprocess.run(['meson', 'configure', f'-D{key}={value}'], check=True, cwd=BUILD_DIR)
    
    return result


def subcmd_compile():
    meson_configure()
    subprocess.run(['meson', 'compile'], check=True, cwd=BUILD_DIR)


def wait_for_device():
    if shutil.which('wchisp') is None:
        sys.exit('Error: wchisp not found! Aborting.')

    attempts = 0
    while attempts < 16:
        try:
            subprocess.run(['meson', 'compile', 'wchisp_info'], check=True, cwd=BUILD_DIR, capture_output=True)
            return
        except subprocess.CalledProcessError:
            if attempts == 0:
                print("Device not available!")
                print("Waiting for bootloader...", end="", flush=True)
            else:
                print(".", end="", flush=True)
            
            time.sleep(1)
            attempts += 1


# TODO: Implement better flashing experience
# - Automatically flash whatever side(s) needs to reflash
def subcmd_flash_central():
    subcmd_compile()
    wait_for_device()
    subprocess.run(['meson', 'compile', 'flash_central'], check=True, cwd=BUILD_DIR)


def subcmd_flash_peripheral():
    result = meson_configure()

    if 'peripheral' not in result:
        print("Error: Can't flash peripheral. The keyboard is not a split.")
        sys.exit(1)

    subcmd_compile()
    wait_for_device()
    subprocess.run(['meson', 'compile', 'flash_peripheral'], check=True, cwd=BUILD_DIR)


def subcmd_load_managed_eval():
    result = json.loads(sys.stdin.read())
    result['__hash__'] = HASH_MANAGED

    with open(EVAL_PATH, 'w') as f:
        f.write(json.dumps(result, indent=2))


def subcmd_clean():
    if os.path.isfile(EVAL_PATH):
        os.remove(EVAL_PATH)
    
    if os.path.isdir(BUILD_DIR):
        shutil.rmtree(BUILD_DIR)
    

# TODO: Use argparse

if SUBCOMMAND == 'query_ncl':
    subcmd_query_ncl()
elif SUBCOMMAND == 'compile':
    subcmd_compile()
elif SUBCOMMAND in ['flash', 'flash_c', 'flash_central']:
    subcmd_flash_central()
elif SUBCOMMAND in ['flash_p', 'flash_peripheral']:
    subcmd_flash_peripheral()
elif SUBCOMMAND == 'load_managed_eval':
    subcmd_load_managed_eval()
elif SUBCOMMAND == 'clean':
    subcmd_clean()
else:
    print("Error: Unknown subcommand")
    sys.exit(1)
