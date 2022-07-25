#!/usr/bin/env python
#coding=utf-8

import os, sys
import shutil

def scan_files(directory, prefix=None, postfix=None):
	files_list=[]
	for root, sub_dirs, files in os.walk(directory):
		for special_file in files:
			if postfix:
				if special_file.endswith(postfix):
					files_list.append([root, os.path.join(root,special_file), special_file])
			elif prefix:
				if special_file.startswith(prefix):
					files_list.append([root, os.path.join(root,special_file), special_file])
			else:
				files_list.append([root, os.path.join(root,special_file), special_file])
	return files_list

def luac(filenameList):
	for value in filenameList:
		root, filename, name = value[0], value[1], value[2] 
		os.system(".\\luabin\\luac -o %s %s"%(os.path.join(root, name + "c"), filename))
		os.remove(filename)
		shutil.move(os.path.join(root, name + "c"), os.path.join(root, name))

if __name__ == "__main__":
	bLinuxpack = False
	if len(sys.argv) > 1 and sys.argv[1] == "linux":
		bLinuxpack = True
	root = ".\\server_luac\\"
	shutil.copytree(".\\luabin\\", root, ignore = shutil.ignore_patterns('*.git', '*.txt'))
	luac(scan_files(root + "logic", postfix = "lua"))
	luac(scan_files(root + "lib", postfix = "lua"))
	luac(scan_files(root + "service", postfix = "lua"))

	#os.system("rd /S /Q %s\\conf"%(root))
	os.system("rd /S /Q %s\\log"%(root))
	os.system("rd /S /Q %s\\service\\shell_linux"%(root))
	os.system("rd /S /Q %s\\service\\shell_win"%(root))
	os.system("del /S /F /Q /A %s\\luaclib\\*.so"%(root))

	os.system("del /S /F /Q /A %s\\liblua.a"%(root))
	os.system("del /S /F /Q /A %s\\lua"%(root))
	os.system("del /S /F /Q /A %s\\luac"%(root))
	os.system("del /S /F /Q /A %s\\protoc.exe"%(root))
	os.system("del /S /F /Q /A %s\\luac.*"%(root))

