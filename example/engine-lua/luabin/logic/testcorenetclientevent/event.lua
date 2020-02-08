local msgpack = require "msgpack53"
local ccorenet = require "ccorenet"
local IClient = require "IClient"
local CoreTool = require "CoreTool"
local ccoroutine = require "ccoroutine"
local CoreTool = require "CoreTool"
local util = require "util"

local client_event = {}

function client_event.OnConnect(IClientClassObj, ip)
	--print("c_connect", IClientClassObj, ip)
	IClientClassObj:SendData("watchdog.", "sssssss", ip)
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

function client_event.OnRecvCommon(IClientClassObj, targetName, proto, data)
	print("OnRecvCommon", IClientClassObj, targetName, proto, data)
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
	ccorenet.addtimer(client_event, "session_id_coroutine_timeout", 1000)
	client_event.framefunc()
	ccorenet.getGlobalObj("clientObj"):SendRemoteData(10000000, 199, "ip.......")
	ccorenet.getGlobalObj("clientObj"):SendData("watchdog.", "sssssss", "asddff")
	local oo, data = ccorenet.getGlobalObj("clientObj"):CallData("watchdog......", "testCallData", {"12345"})
	--print(oo)
	--if type(data) == type({}) then 
	--	util.print(data) 
	--else 
	--	print(data) 
	--end
end

ccorenet.addtimer(client_event, "session_id_coroutine_timeout", 1000)

local domain = ccorenet.IpV4
local ip = (ccorenet.getOS() == "Linux" and domain == ccorenet.UnixLocal) and "dont.del.local.socket" or "127.0.0.1"
local clientObj = IClient.new("clientObj", client_event, ip, 8888, 1000*60, 5*1000, domain, false, false)
ccorenet.addGlobalObj(clientObj, clientObj:GetName())

return client_event