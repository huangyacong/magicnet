local AgentService = require "AgentService"
local ccoroutine = require "ccoroutine"
local net_module = require "ccorenet"
local util = require "util"
local timer = require "timer"
require "class"

local local_modulename = ...
timer.register(local_modulename)

local event = {}
function event.OnRecvCall(proto, data)
	print("OnRecvCall", proto, data)
	--AgentService.RetCallData("back....")
	--local socket = table.unpack(data)
	--AgentService.SendRemote(socket, 456, "sssss")
	AgentService.RetCallData({1,2,3,4,5,6}, true)
	AgentService.RetCallData({1,2,3,4,5,6, 999}, true)
	AgentService.RetCallData(data)
end

function event.OnRecvCommon(proto, data)
	print("OnRecvCommon", proto, data)
end

function event.OnSystem(proto, data)
	print(proto, data)
end

function event.OnRegister()
end

function event.framefunc()
	timer.addtimer(local_modulename, "framefunc", 10000)

	local a, b,c = AgentService.CallData(".watchdog", "ssss", {1,2,3, "22222"})
	print("sssssssssssssssssssssss")
	util.print(table.pack(a, b,c))
end

return event
