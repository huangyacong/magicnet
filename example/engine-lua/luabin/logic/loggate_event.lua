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

	local a, b,c = AgentService.CallData("svr_a", "ssss", {1,2,3, "22222"})
	print("sssssssssssssssssssssss")
	util.print(table.pack(a, b,c))


end

return event
