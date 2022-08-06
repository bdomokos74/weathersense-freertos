#import subprocess
Import("env")
import shutil
import os

def copyFirmware(source, target, env):
    tgt = target[0].get_abspath()
    board = tgt.split(os.sep)[-2]
    with open('version.txt', 'r') as file:
        firmwareVersion = file.read().strip()
        fname = f"tmp/firmware_{firmwareVersion}_{board}.bin"
        print("copy:", fname)
        shutil.copyfile(tgt ,fname)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copyFirmware)
