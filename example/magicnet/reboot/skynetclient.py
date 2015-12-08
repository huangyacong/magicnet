#!/usr/bin/env python
# -*- coding: utf-8 -*-
#======================================================================
#
# skynetclient.py - skynetclient network data stream operation interface
#
# NOTE: The Replacement of skynetclient 
#
#======================================================================

import msgpack
import socket
import select
import struct
import errno
import copy
import time
import sys

SKYNET_CLIENT_STATE_STOP = 0			# state: init value
SKYNET_CLIENT_STATE_CONNECTING = 1		# state: connecting
SKYNET_CLIENT_STATE_ESTABLISHED = 2		# state: connected
SKYNET_CLIENT_STATE_RUNNING = 3			# state: running
SKYNET_CLIENT_STATE_DISCONNECTING = 4	# state: disconnecting

CONNECT_TIME_OUT = 5					# connect time out

#======================================================================
# skynetclient - basic tcp stream
#======================================================================
class skynetclient(object):
	
	def __init__(self, autoconnect = True):
		self.sock = None		# socket object
		self.send_buf = ''		# send buffer
		self.recv_buf = ''		# recv buffer
		self.state = SKYNET_CLIENT_STATE_STOP
		self.errd = ( errno.EINPROGRESS, errno.EALREADY, errno.EWOULDBLOCK )
		self.conn = ( errno.EISCONN, 10057, 10053 )
		self.errc = 0
		self.address = ''
		self.port = 0
		self.autoconnect = autoconnect
		self.time = 0

	def __try_connect(self):
		try:
			self.sock.recv(0)
		except socket.error, (code, strerror):
			if code in self.conn:
				return 0
			if code in self.errd:
				self.state = SKYNET_CLIENT_STATE_ESTABLISHED
				self.recv_buf = ''
				return 1
			self.__close()
			return -1
		self.state = SKYNET_CLIENT_STATE_ESTABLISHED
		return 1

	# try to receive all the data into recv_buf
	def __try_recv(self):
		rdata = ''
		while 1:
			text = ''
			try:
				text = self.sock.recv(1024)
				if not text:
					self.errc = 10000
					return -1
			except socket.error,(code, strerror):
				if not code in self.errd:
					#sys.stderr.write('[TRYRECV] '+strerror+'\n')
					self.errc = code
					return -1
			if text == '':
				break
			rdata += text
		self.recv_buf += rdata
		return len(rdata)

	# send data from send_buf until block (reached system buffer limit)
	def __try_send(self):
		wsize = 0
		if (len(self.send_buf) == 0):
			return 0
		try:
			wsize = self.sock.send(self.send_buf)
		except socket.error,(code, strerror):
			if not code in self.errd:
				#sys.stderr.write('[TRYSEND] '+strerror+'\n')
				self.errc = code
				return -1
		self.send_buf = self.send_buf[wsize:]
		return wsize

	# connect the remote server
	def connect(self, address, port):
		if self.state != SKYNET_CLIENT_STATE_STOP:
			return 1
		self.time = int(time.time())
		self.address = address
		self.port = port
		self.__close()
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.setblocking(0)
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
		self.sock.connect_ex((self.address, self.port))
		self.state = SKYNET_CLIENT_STATE_CONNECTING
		self.send_buf = ''
		self.recv_buf = ''
		self.errc = 0
		return 0

	# close connection
	def close(self):
		if self.state == SKYNET_CLIENT_STATE_RUNNING:
			self.state = SKYNET_CLIENT_STATE_DISCONNECTING
		else:
			return 0
		try:
			self.sock.close()
		except:
			pass
		self.sock = None
		return 0
	
	def __close(self):
		try:
			self.sock.close()
		except:
			pass
		self.sock = None
		self.state = SKYNET_CLIENT_STATE_STOP
		return 0
	
	# update 
	def process(self, onconnect, ondisconnect):
		if self.state == SKYNET_CLIENT_STATE_CONNECTING:
			if int(time.time()) - self.time > CONNECT_TIME_OUT:
				self.__close()
				return 0
			self.__try_connect()
			return 0
		if self.state == SKYNET_CLIENT_STATE_ESTABLISHED:
			self.state = SKYNET_CLIENT_STATE_RUNNING
			onconnect()
			return 0
		if self.state == SKYNET_CLIENT_STATE_RUNNING:
			if self.__try_recv() < 0:
				self.close()
				return 0
			if self.__try_send() < 0:
				self.close()
				return 0
			return 0
		if self.state == SKYNET_CLIENT_STATE_DISCONNECTING:
			self.state = SKYNET_CLIENT_STATE_STOP
			ondisconnect()
			return 0
		if self.state == SKYNET_CLIENT_STATE_STOP and self.autoconnect == True:
			self.__close()
			self.connect(self.address, self.port)
			return 0
		return 0
	
	# return state
	def status(self):
		return self.state
	
	# append data into send_buf 
	def send(self, sendData):
		data = msgpack.packb(sendData)
		if self.state != SKYNET_CLIENT_STATE_RUNNING:
			return 0
		packet = struct.pack('>H', len(data)) + data
		self.send_buf += packet
		return 0
	
	# recv an entire message from recv_buf
	def recv(self):
		if self.state != SKYNET_CLIENT_STATE_RUNNING:
			return None

		packetLen = struct.calcsize('>H')

		# 看看是否与足够的数据
		if len(self.recv_buf) < packetLen:
			return None
		
		# 解释包的长度
		datalen, = struct.unpack('>H', self.recv_buf[0:packetLen])

		# 解释data
		if len(self.recv_buf) < (packetLen + datalen):
			return None
		data = self.recv_buf[packetLen:packetLen + datalen]
		
		# 删除数据
		self.recv_buf = self.recv_buf[packetLen + datalen:]
		return msgpack.unpackb(data)

#----------------------------------------------------------------------
# TCPClient
#----------------------------------------------------------------------
class client(skynetclient):
	def __init__(self, autoconnect = False):
		super(client, self).__init__(autoconnect)

	def process(self, onconnect, disconnect, update):
		super(client, self).process(onconnect, disconnect)
		recv_data = self.recv()
		if recv_data == None:
			return
		update(recv_data)
		pass
