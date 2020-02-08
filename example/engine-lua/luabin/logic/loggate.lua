local AgentGate = require "AgentGate"
local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local util = require "util"

local event = {}

function event.OnLocalRecvCall(IServerObj, socket, proto, data)
	print("OnLocalRecvCall", IServerObj, socket, proto, data)
	IServerObj:RetCallData(socket, data)
end

function event.OnLocalRecvCommon(proto, data)
	print("OnLocalRecvCommon", proto, data)
end

function event.OnRemoteRecv(socket, proto, data)
	print("OnRemoteRecv", socket, proto, data)
	AgentGate.SendRemote(socket, 7999, "dddddddddddddddddddddd")

	local ret, data = AgentGate:CallData("svr_a", "func_test", table.pack(socket, "call test"), 2000)
	util.print(table.pack(ret, data))
	print("ddddddddddddddddddddsssssssssssssssssssssssssssss")
end

function event.OnRemoteConnect(socket)
	print("OnRemoteConnect", socket)
end

function event.OnRemoteDisConnect(socket)
	print("OnRemoteDisConnect", socket)
end

function event.framefunc()
	net_module.addtimer(event, "framefunc", 1000)
	--AgentGate.SendData("svr_a", "aaaaaFunction", "test data...")
	--local ret, data = AgentGate:CallData("svr_a", "func_test", "call test", 2000)
	--util.print(table.pack(ret, data))
	--print("ddddddddddddddddddddsssssssssssssssssssssssssssss")
end

net_module.init("./test_svr.log", 65535, true)
net_module.addtimer(event, "framefunc", 1000)

AgentGate.Init("gate", event, "127.0.0.1", 8888, 60000, "127.0.0.1", 6666, "sssssss", 60000, false)

net_module.start()

net_module.fin()
