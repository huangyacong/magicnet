local magicnet = require "magicnet"

local pcLogName, iSocketTimeOut, iSvrPort = "watchdog", 60*1000, 6666
local result = magicnet.SvrInit(pcLogName, iSocketTimeOut, iSvrPort, 0)

if not result then
	assert(false, "watchdog init failed!")
end

magicnet.RegSvr("watchdog.")

while true do
	
	local function work(event, hid, data)

		if event == magicnet.MAGIC_CLIENT_CONNECT then
			print(string.format("client connect hid=%s data=%s len=%s", hid, data, string.len(data)))
			magicnet.SvrSendSvr("game", "watchdog to game data")
			magicnet.SvrBindClient(hid, "game")
			--magicnet.SvrCloseClient(hid)
		end

		if event == magicnet.MAGIC_CLIENT_DISCONNECT then
			print(string.format("client disconnect hid=%s data=%s len=%s", hid, data, string.len(data)))
		end

		if event == magicnet.MAGIC_RECV_DATA_FROM_SVR then
			print(string.format("recv data from svr hid=%s data=%s len=%s", hid, data, string.len(data)))
		end

		if event == magicnet.MAGIC_RECV_DATA_FROM_CLIENT then
			print(string.format("recv data from client hid=%s data=%s len=%s", hid, data, string.len(data)))
			magicnet.SvrSendClient(hid, data)
		end

	end
	
	local nowEvent = nil
	local recvData = magicnet.SvrRead(1000)
	
	for _, oneValue in ipairs(recvData) do
		local netevent, nethid, netdata = oneValue[1], oneValue[2], oneValue[3]

		if netevent == magicnet.MAGIC_SHUTDOWN_SVR or netevent == magicnet.MAGIC_IDLE_SVR_DATA then
			nowEvent = netevent
			break
		else
			local isOK, ret = pcall(function () return work(netevent, nethid, netdata) end)
			if not isOK then print(ret) end
		end

	end

	if nowEvent == magicnet.MAGIC_SHUTDOWN_SVR then break end
	if nowEvent == magicnet.MAGIC_IDLE_SVR_DATA then magicnet.TimeSleep(1) end

end

print("watchdog exit")

magicnet.SvrFin()
