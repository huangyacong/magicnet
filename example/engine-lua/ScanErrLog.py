#!/usr/bin/env python
#coding=utf-8

######
## 扫描错误日志
######

import os
import sys
from shutil import copyfile

def scan_files(directory,prefix=None,postfix=None):
	files_list=[]
	for root, sub_dirs, files in os.walk(directory):
		for special_file in files:
			if postfix:
				if special_file.endswith(postfix):
					files_list.append(os.path.join(root,special_file))
			elif prefix:
				if special_file.startswith(prefix):
					files_list.append(os.path.join(root,special_file))
			else:
				files_list.append([os.path.join(root,special_file), special_file])
	return files_list

# 读取 txt 文件，返回文件内容 
def readTxt(fileUrl):
	out = {}
	print(fileUrl)
	#with open(fileUrl, 'r', encoding='UTF-8', errors="ignore") as f:
	with open(fileUrl, 'r') as f:
		data = []
		for row in f:
			data.append(row)

		step, totalCount = 0, len(data)

		bFind, result = False, ""
		while step < totalCount:
			row = data[step]
			step = step + 1

			if not bFind:
				index = row.find("stack traceback")
				if index > 0:
					bFind = True
				continue

			if bFind:
				if not row[0].isdigit():
					result = result + row
				if row[0].isdigit():
					if result.find("kick same player and be suspend") < 0 and result.find("MySQL server has gone away") < 0:
						out[result] = [fileUrl, step]
					result = ""
					bFind = False
					if  row.find("stack traceback") > 0:
						bFind = True

		if len(result) > 0:
			out[result] = [fileUrl, step]

	return out

sanPath = os.path.join(os.path.abspath(sys.path[0]), ".\\luabin\\log\\")
files_list = scan_files(sanPath, postfix="log")

filePath = os.path.join(os.path.abspath(sys.path[0]), ".\\traceback.txt")
file = open(filePath, "w")

for pathstr in files_list:
	ddd = readTxt(pathstr)
	for value, key in ddd.items():
		file.write(value)
		file.write('[%s] line=[%s]\n\n'%(key[0], key[1]))

file.close()
