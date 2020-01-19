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
	IClientClassObj:SendData("sssssss", ip)
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

function client_event.OnRecv(IClientClassObj, proto, data)
	--print("c_recv", IClientClassObj, proto, data)
	--
	if "testCallData" == proto then
		IClientClassObj:RetCallData(data)
		return
	end
	IClientClassObj:SendData(proto, data)
	--print("c_recv", IClientClassObj, proto, data)
end

local timero = CoreTool.GetTickCount()
function client_event.framefunc()
	--if CoreTool.GetTickCount() - timero < 1000 *17 then
	
	--end
	ccorenet.getGlobalObj("clientObj"):TryReConnect()
	--ccorenet.getGlobalObj("clientObj"):TimeToPingPing()
	
	--if 0 == 0 then return end
	--print("framefuncddddddddddddd")
	local report = ccorenet.statreport()
	--util.print_table(ccorenet.IClientList)
	if report then
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed = table.unpack(report)
		print(os.date("%Y-%m-%d %H:%M:%S"), string.format("statreport sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s %s, %s", sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine()))
		collectgarbage()
	end

	
end

function client_event.session_id_coroutine_timeout()
	ccorenet.addtimer(client_event, "session_id_coroutine_timeout", 1)
	local oo, data = ccorenet.getGlobalObj("clientObj"):CallData("testCallData", {"12345"})
	--print(oo)
	--if type(data) == type({}) then 
	--	util.print(data) 
	--else 
	--	print(data) 
	--end
end

ccorenet.addtimer(client_event, "session_id_coroutine_timeout", 1)
local clientObj = IClient.new("clientObj", ccorenet, client_event, "127.0.0.1", 8888, 1000*60, 5*1000, false, false)
ccorenet.addGlobalObj(clientObj, clientObj:GetName())

return client_event