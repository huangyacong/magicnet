#!/usr/bin/env python
# -*- coding: utf-8 -*-
#======================================================================
#
# reboot.py - reboot network data stream operation interface
#
# NOTE: The Replacement of reboot 
#
#======================================================================

import sys
import time
import random
from skynetclient import client

num = {"num":1, 'dis':1, 'time':time.time()}
PROTOCOL_VERSIONS_NO = 7
class reboot(client):
	def __init__(self, autoconnect = False):
		super(reboot, self).__init__(autoconnect)
		self.send_time = int(time.time())
		self.username = ''
		self.password = ''
		self.playerid = 0
		self.g = 0
		self.in_scene = False

	def send(self, sendData):
		super(reboot, self).send(sendData)
		self.send_time = int(time.time())

	def send_to_server(self, msg_no, param):
		self.g += 1
		self.send({'g':self.g,'m':msg_no, 'p':param })

	def connect_svr(self, addr, port, username, password):
		self.connect(addr, port)
		self.username = username
		self.password = password

	def process(self):
		super(reboot, self).process(self.onconnect, self.disconnect, self.recv_data)
		if int(time.time()) - self.send_time > 10:
			# 心跳
			self.send({'g':0,'m':500, 'p':{}})
			pass
		pass	

	def onconnect(self):
		num['dis'] += 1
		#print 'onconnect username=%s'%(self.username)
		# 账号验证
		self.send_to_server(5002, {'plat':0, 'token':self.username})

		
	def disconnect(self):
		num['dis'] -= 1
		print 'disconnect username=%s,%s'%(self.username, num['dis'])
		
	def recv_data(self, data):
		print u'recv username=%s'%(self.username), data
		self.send_to_server(5002, {'plat':0, 'token':self.username})

	 


#----------------------------------------------------------------------
# reboot case
#----------------------------------------------------------------------

if __name__ == '__main__':
	print("use args python reboot.py addr port reboot_count")
	addr, port, count = '127.0.0.1', 8888, 1
	if len(sys.argv) > 1:
		addr = str(sys.argv[1])
		port = int(sys.argv[2])
		count = int(sys.argv[3])
	print("now start %d reboots addr=%s port=%d"%(count, addr, port))
	
	# 机器人存储
	rebootApp = {}
	
	# 机器人初始化
	for i in xrange(count):
		rebootApp[i] = reboot(True)
		rebootApp[i].connect_svr(addr, port, 'test%d'%(i + 1), '123456')
	
	# 机器人循环做事
	while 1:
		for i in xrange(count):
			rebootApp[i].process()
		pass
