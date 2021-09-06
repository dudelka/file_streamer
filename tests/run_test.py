#!/usr/bin/env python3

import pathlib
import os
import subprocess
import shlex

def get_files_from_dir(dirname):
    result = []
    filenames = os.listdir(dirname)
    for filename in filenames:
        result.append(dirname + '/' + filename)
    return result

def add_option(command, option, value):
    return command + ' ' + option + ' ' + value

def remove_file(filename):
    if os.path.isfile(filename):
        os.remove(filename)

def check_log(log_info):
    if log_info.find('Mismatched') != -1:
        return False
    return True

def write_to_file(filename, info):
    with open(filename, 'x') as f:
        f.write(info)

def run_test(curdir, files_for_test):
    client_bin = curdir + '/bindir/client'
    client_in = '127.0.0.1:10505'
    client_out = '127.0.0.1:11504'
    client_cmd = add_option(client_bin, '--send_address', client_out)
    client_cmd = add_option(client_cmd, '--receive_address', client_in)
    for filename in files_for_test:
        client_cmd = add_option(client_cmd, '--file', filename)

    server_bin = curdir + '/bindir/server'
    server_in = client_out
    server_out = client_in
    server_cmd = add_option(server_bin, '--send_address', server_out)
    server_cmd = add_option(server_cmd, '--receive_address', server_in)
    server_cmd = add_option(server_cmd, '--timeout', '1')

    for i in range(5):
        print('=========================')
        print('Test number ' + str(i))
        client_proc = subprocess.Popen(client_cmd, stdout = subprocess.PIPE, 
                stderr = subprocess.PIPE, shell = True, encoding = 'utf-8')
        server_proc = subprocess.Popen(server_cmd, stdout = subprocess.PIPE, 
                stderr = subprocess.PIPE, shell = True, encoding = 'utf-8')
        client_stdout, client_stderr = client_proc.communicate()
        server_stdout, server_stderr = server_proc.communicate()
        server_proc.wait()
        if not check_log(client_stdout):
            print('TEST FAILED')
            print('Check logs for additional info')
            write_to_file(curdir + '/client_stdout', client_stdout)
            write_to_file(curdir + '/client_stderr', client_stderr)
            write_to_file(curdir + '/server_stdout', server_stdout)
            write_to_file(curdir + '/server_stderr', server_stderr)
            os._exit(os.EX_IOERR)
    print('=========================')
    print('TEST SUCCEEDED')

def main():
    curdir = str(pathlib.Path(__file__).parent.resolve())
    files_for_test = get_files_from_dir(curdir + '/test_files')
    remove_file(curdir + '/client_stdout')
    remove_file(curdir + '/client_stderr')
    remove_file(curdir + '/server_stdout')
    remove_file(curdir + '/server_stderr')
    run_test(curdir, files_for_test)

if __name__ == '__main__':
    main()
