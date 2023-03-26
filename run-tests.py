import subprocess
import os
import difflib
from itertools import combinations
import re

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    
params = ['-r', '-p', '-u', '-a', '-g'];
paramsCombos = sum([list(map(list, combinations(params, i))) for i in range(len(params) + 1)], [])
errorFiles = open('test_results.txt', 'w')

total = 0;
errors = 0;


def build():
	os.system("cd tests && gcc hello_world.c -c -o out/hello_world.o")
	os.system("cd tests && gcc hello_world.c -o out/hello_world")
	os.system("cd tests && gcc libary.c -c -o out/libary.o")
	os.system("cd tests && gcc out/libary.o out/hello_world.o -shared -o out/shared.so")

	os.system("cd tests && gcc -m32 hello_world.c -c -o out/hello_world_32.o")
	os.system("cd tests && gcc -m32 hello_world.c -m32 -c -o out/hello_world_32.o")
	os.system("cd tests && gcc -m32 hello_world.c -o out/hello_world_32")
	os.system("cd tests && gcc -m32 libary.c -c -o out/libary_32.o")
	os.system("cd tests && gcc -m32 out/libary.o out/hello_world.o -shared -o out/shared_32.so")

def check(command):
	global total, errors

	total += 1;
	res = subprocess.run(f"/bin/bash -c \"diff <(./ft_nm {command}) <(nm {command})\"", stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
	if (res.returncode != 0):
		print(f"[{bcolors.FAIL}KO{bcolors.ENDC}]");
		errorFiles.write(f"\ndiff <(./ft_nm {command} 2> ./out) <(nm {command})\n");
		errorFiles.write(res.stdout.decode("utf-8"))
		errorFiles.write("\n-STDERR-\n" + res.stderr.decode("utf-8") + "--")
		errors += 1;
	else:
		print(f"[{bcolors.OKGREEN}OK{bcolors.ENDC}]");
def clear():
	os.system("rm -rf tests/out/*")

def run_os_full():
	all_files = subprocess.run("find / -type f -name *.o", stdout=subprocess.PIPE, shell=True).stdout.decode('utf-8').split('\n') + \
				subprocess.run("find / -type f -name *.so", stdout=subprocess.PIPE, shell=True).stdout.decode('utf-8').split('\n') + \
				subprocess.run("find / -type f -executable", stdout=subprocess.PIPE, shell=True).stdout.decode('utf-8').split('\n')
	for filename in all_files:
		if (os.path.isfile(filename)):
			for paramCombo in paramsCombos:
				print(f"[{bcolors.UNDERLINE}TEST{bcolors.ENDC}] ## {bcolors.OKCYAN} {' '.join(paramCombo)} {filename} {bcolors.ENDC} ## ", end='')
				check(f" {' '.join(paramCombo)} {filename}")

def run_nm():
	for paramCombo in paramsCombos:
		print(f"[{bcolors.UNDERLINE}TEST{bcolors.ENDC}] ## {bcolors.OKCYAN} {' '.join(paramCombo)} ./ft_nm {bcolors.ENDC} ## ", end='')
		check(f"{' '.join(paramCombo)} ./ft_nm")

def run_tests():
	for filename in os.listdir('./tests/out'):
		f = os.path.join('./tests/out', filename)
		# checking if it is a file
		if os.path.isfile(f):
			for paramCombo in paramsCombos:
				print(f"[{bcolors.UNDERLINE}TEST{bcolors.ENDC}] ## {bcolors.OKCYAN} {' '.join(paramCombo)} {f} {bcolors.ENDC} ## ", end='')
				check(f" {' '.join(paramCombo)} {f}")


build();
run_tests();

print("ok lets go")
	

print(f"{total - errors} out of {total} ran without errors. See {bcolors.UNDERLINE}test_results.txt{bcolors.ENDC} for the errors");

# if (total == errors):
# 	clear()