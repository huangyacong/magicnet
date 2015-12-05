import magicnet
import traceback

pcLogName, iSocketTimeOut, iSvrPort = "game", 30*1000, 6666
result = magicnet.SvrInit(pcLogName, iSocketTimeOut, iSvrPort)

if not result:
	raise "game inti failed!"

magicnet.RegSvr("game")

while True:
	try:
		event, hid, data = magicnet.SvrRead()
		if event == magicnet.MAGIC_SHUTDOWN_SVR:
			break
		if event == magicnet.MAGIC_IDLE_SVR_DATA:
			continue
		if event == magicnet.MAGIC_CLIENT_CONNECT:
			print("client connect hid=%d data=%s len=%s"%(hid, data, len(data)))
		if event == magicnet.MAGIC_CLIENT_DISCONNECT:
			print("client disconnect hid=%d data=%s len=%s"%(hid, data, len(data)))
		if event == magicnet.MAGIC_RECV_DATA_FROM_SVR:
			print("recv data from svr hid=%d data=%s len=%s"%(hid, data, len(data)))
			magicnet.SvrSendSvr("watchdog.", "game to game data")
		if event == magicnet.MAGIC_RECV_DATA_FROM_CLIENT:
			print("recv data from client hid=%d data=%s len=%s"%(hid, data, len(data)))
			magicnet.SvrSendClient(hid, data)
	except:
		print(traceback.format_exc())

magicnet.SvrFin()
