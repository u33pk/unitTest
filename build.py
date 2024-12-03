import subprocess
import os

def exec_cmd(cmd):
    print(cmd)
    res = []
    subp = subprocess.Popen(cmd, shell=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    for line in iter(subp.stdout.readline, b''):
        try:
            res.append(line.decode('utf-8').strip())
            # sys.stdout.flush()
        except Exception as e:
            continue
    
    return res

def get_allJS():
    cmd = "find ./isa -type f -name '*.js'"
    return exec_cmd(cmd)

def build():
    for f in get_allJS():
        name, _ = os.path.splitext(os.path.basename(f))
        cmd ="./es2abc {_in} --output out/{_out}.abc".format(_in = f, _out = name)
        exec_cmd(cmd)

if __name__ == '__main__':
    build()