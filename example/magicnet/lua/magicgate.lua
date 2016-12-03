local magicnet = require "magicnet"

--[[
MagicGateInit = magicnet.GateInit
MagicGateFin = magicnet.GateFin
MagicGateProcess = magicnet.GateProcess

MAGIC_SHUTDOWN_SVR = magicnet.MAGIC_SHUTDOWN_SVR
MAGIC_IDLE_SVR_DATA = magicnet.MAGIC_IDLE_SVR_DATA
MAGIC_CLIENT_CONNECT = magicnet.MAGIC_CLIENT_CONNECT
MAGIC_CLIENT_DISCONNECT = magicnet.MAGIC_CLIENT_DISCONNECT
MAGIC_RECV_DATA_FROM_SVR = magicnet.MAGIC_RECV_DATA_FROM_SVR
MAGIC_RECV_DATA_FROM_CLIENT = magicnet.MAGIC_RECV_DATA_FROM_CLIENT

MagicSvrInit = magicnet.SvrInit
MagicSvrFin = magicnet.SvrFin
MagicRegSvr = magicnet.RegSvr
MagicSvrSendClient = magicnet.SvrSendClient
MagicSvrBindClient = magicnet.SvrBindClient
MagicSvrCloseClient = magicnet.SvrCloseClient
MagicSvrSendSvr = magicnet.SvrSendSvr
MagicSvrRead = magicnet.SvrRead
--]]

local pcLogName, iSocketTimeOut, iMaxConnect, iGatePort, iSvrPort = "gate", 60*1000, 10000, 8888, 6666
local result = magicnet.GateInit(pcLogName, iSocketTimeOut, iMaxConnect, iGatePort, iSvrPort, 0)

if not result then
	assert(false, "gate init failed!")
end

while true do
	magicnet.GateProcess()
end

magicnet.GateFin()
