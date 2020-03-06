local AgentService = require "AgentService"
local ccoroutine = require "ccoroutine"
local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local util = require "util"
local timer = require "timer"

local local_modulename = ...
timer.register(local_modulename)

local event = {}


local client_list = {}
local count = 0
function event.OnRecvCall(proto, data)
	print("OnRecvCall", proto, data)
	AgentService.RetCallData(data)
end

function event.OnRecvCommon(proto, data)
	print("OnRecvCommon", proto, data)
end

function event.OnRegister()
	
end

function event.OnRemoteRecvData(socket, proto, data)
	print("OnRemoteRecvData",socket, proto, data)
	AgentService.SendRemote(socket, proto, data)
end

function event.OnRemoteConnect(socket, ip)
client_list[socket] =socket
count = count + 1
print("OnRemoteConnect",socket, ip)
AgentService.CloseRemote(socket)
end

function event.OnRemoteDisConnect(socket)
	client_list[socket] =nil
	count = count - 1
	print("OnRemoteDisConnect",socket)
end

function event.framefunc()
	timer.addtimer(local_modulename, "framefunc", 10000)

	local a, b = AgentService.CallData("svr_a", "ssss", {1,2,3})
	util.print(table.pack(a, b))

	AgentService.SendData("svr_a", "ssss", {1,2,3})


	local a, b = AgentService.CallData(".watchdog", "me call", {1,2,3})
	util.print(table.pack(a, b))

	AgentService.SendData(".watchdog", "me common", {1,2,3})

	local report = net_module.statReport((net_module.getOS() ~= "Linux") and (30 * 1000) or 0)
	--util.print_table(ccorenet.IClientList)
	if report then
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount = table.unpack(report)
		print(os.date("%Y-%m-%d %H:%M:%S"), string.format("statreport count=%s sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s timerCount=%s %s, %s pool=%s", 
				count, sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine(), ccoroutine.count_coroutine_pool()))
		collectgarbage()
	end
end

return event
