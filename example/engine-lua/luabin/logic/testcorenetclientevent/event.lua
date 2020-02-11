local msgpack = require "msgpack53"
local ccorenet = require "ccorenet"
local IClientPlayer = require "IClientPlayer"
local CoreTool = require "CoreTool"
local ccoroutine = require "ccoroutine"
local CoreTool = require "CoreTool"
local util = require "util"

local client_event = {}

function client_event.OnConnect(IClientClassObj, ip)
	--print("c_connect", IClientClassObj, ip)
	--IClientClassObj:SendData("watchdog.", "sssssss", ip)
end

function client_event.OnConnectFailed(IClientClassObj)
	--print("OnConnectFailed", IClientClassObj)
end

function client_event.OnDisConnect(IClientClassObj)
	--print("c_disconnect", IClientClassObj)
end

function client_event.OnSendPacketAttach(IClientClassObj)
	--print("OnSendPacketAttach", IClientClassObj)
end

function client_event.OnPing(IClientClassObj)
	--print("OnPing", IClientClassObj)
end

function client_event.OnRecvCall(IClientClassObj, targetName, proto, data)
	print("OnRecvCall", IClientClassObj, targetName, proto, data)
	--
	if "testCallData" == proto then
		IClientClassObj:RetCallData(data)
		return
	end
end

function client_event.OnRecv(IClientClassObj, proto, data)
	--print("OnRecvCommon", IClientClassObj, proto, data)
end

local timero = CoreTool.GetTickCount()
function client_event.framefunc()
	--if CoreTool.GetTickCount() - timero < 1000 *17 then
	
	--end
	--ccorenet.getGlobalObj("clientObj"):TryReConnect()
	--ccorenet.getGlobalObj("clientObj"):TimeToPingPing()
	
	--if 0 == 0 then return end
	--print("framefuncddddddddddddd")
	local report = ccorenet.statreport()
	--util.print_table(ccorenet.IClientList)
	if report then
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount = table.unpack(report)
		print(os.date("%Y-%m-%d %H:%M:%S"), string.format("statreport sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s timerCount=%s %s, %s pool=%s", 
				sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine(), ccoroutine.count_coroutine_pool()))
		collectgarbage()
	end

	
end

function client_event.session_id_coroutine_timeout()
	ccorenet.addtimer(client_event, "session_id_coroutine_timeout", 500)
	client_event.framefunc()
	--ccorenet.getGlobalObj("clientObj"):SendRemoteData(10000000, 199, "ip.......")
	for i = 0, 5000 do
	local name = string.format("clientObj%s", i)
	--for oo = 0, 1000 do
	ccorenet.getGlobalObj(name):SendData(45566, "asddff")
--end
	end
	--local oo, data = ccorenet.getGlobalObj("clientObj"):CallData("watchdog......", "testCallData", {"12345"})
	--print(oo)
	--if type(data) == type({}) then 
	--	util.print(data) 
	--else 
	--	print(data) 
	--end
end

ccorenet.addtimer(client_event, "session_id_coroutine_timeout", 10000)

local domain = ccorenet.IpV4
local ip = (ccorenet.getOS() == "Linux" and domain == ccorenet.UnixLocal) and "dont.del.local.socket" or "127.0.0.1"
for i = 0, 5000 do
	local name = string.format("clientObj%s", i)
local clientObj = IClientPlayer.new(name, client_event, ip, 8888, 1000*60, 5*1000, true, false)
ccorenet.addGlobalObj(clientObj, clientObj:GetName())
end

return client_event