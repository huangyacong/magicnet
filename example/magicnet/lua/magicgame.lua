local magicnet = require "magicnet"

local pcLogName, iSocketTimeOut, iSvrPort = "game", 30*1000, 6666
local result = magicnet.SvrInit(pcLogName, iSocketTimeOut, iSvrPort)

if not result then
	assert(false, "game inti failed!")
end

magicnet.RegSvr("game")

while true do
	
	local function work(event, hid, data)

		if event == magicnet.MAGIC_CLIENT_CONNECT then
			print(string.format("client connect hid=%d data=%s len=%s", hid, data, len(data)))
		end

		if event == magicnet.MAGIC_CLIENT_DISCONNECT then
			print(string.format("client disconnect hid=%d data=%s len=%s", hid, data, len(data)))
		end

		if event == magicnet.MAGIC_RECV_DATA_FROM_SVR then
			print(string.format("recv data from svr hid=%d data=%s len=%s", hid, data, len(data)))
			magicnet.SvrSendSvr("watchdog.", "game to watchdog data")
		end

		if event == magicnet.MAGIC_RECV_DATA_FROM_CLIENT then
			print(string.format("recv data from client hid=%d data=%s len=%s", hid, data, len(data)))
			magicnet.SvrSendClient(hid, data)
		end

	end

	local event, hid, data = magicnet.SvrRead()

	if event == magicnet.MAGIC_SHUTDOWN_SVR then break end

	if event ~= magicnet.MAGIC_IDLE_SVR_DATA then
		local isOK, ret = pcall(function () return work(event, hid, data) end)
		if not isOK then print(ret) end
	end

end

magicnet.SvrFin()
