local msgpack = require "msgpack53"
local ccorenet = require "ccorenet"

local CoreTool = require "CoreTool"
local ccoroutine = require "ccoroutine"
local CoreTool = require "CoreTool"
local util = require "util"
local timer = require "timer"

local local_modulename = ...
timer.register(local_modulename)

local client_event = {}

function client_event.OnConnect(IClientClassObj, ip)
	--print("c_connect", IClientClassObj, ip)
	--IClientClassObj:SendSystemData("xxxxxxxxxxxxxxxx", ip)
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

function client_event.OnSystem(IClientClassObj, proto, ret)
	print("OnSystem", proto, ret)
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
	timer.addtimer(local_modulename, "session_id_coroutine_timeout", 500)
	client_event.framefunc()
	--ccorenet.getGlobalObj("clientObj"):SendRemoteData(10000000, 199, "ip.......")
	for i = 0, 0 do
		local name = string.format("clientObj%s", i)
		--for oo = 0, 1000 do
		ccorenet.getGlobalObj(name):SendSystemData("xxxxxxxxxxxxxxxx", "127.0.0.1")
		--end
	
		local oo, data = ccorenet.getGlobalObj(name):CallData("watchdog......", "testCallData", {"12345"})
		print(oo)
		if type(data) == type({}) then 
			util.print(data) 
		else 
			print(data) 
		end
	end
end

return client_event