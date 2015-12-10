import magicnet
import traceback

pcLogName, iSocketTimeOut, iSvrPort = "game", 60*1000, 6666
result = magicnet.SvrInit(pcLogName, iSocketTimeOut, iSvrPort)

if not result:
	raise "game inti failed!"

magicnet.RegSvr("game")

while True:

	try:
		nowEvent = None
		recvList = magicnet.SvrRead(1000)

		for value in recvList:
			event, hid, data = value
			if event == magicnet.MAGIC_SHUTDOWN_SVR or event == magicnet.MAGIC_IDLE_SVR_DATA:
				nowEvent = event
				break
			if event == magicnet.MAGIC_CLIENT_CONNECT:
				print("client connect hid=%s data=%s len=%s"%(hid, data, len(data)))
			if event == magicnet.MAGIC_CLIENT_DISCONNECT:
				print("client disconnect hid=%s data=%s len=%s"%(hid, data, len(data)))
			if event == magicnet.MAGIC_RECV_DATA_FROM_SVR:
				print("recv data from svr hid=%s data=%s len=%s"%(hid, data, len(data)))
			if event == magicnet.MAGIC_RECV_DATA_FROM_CLIENT:
				print("recv data from client hid=%s data=%s len=%s"%(hid, data, len(data)))

		if nowEvent == magicnet.MAGIC_SHUTDOWN_SVR:
			break
		if nowEvent == magicnet.MAGIC_IDLE_SVR_DATA:
			time.sleep(0.001)
	except:
		print(traceback.format_exc())

print("game exit")

magicnet.SvrFin()
