local AgentGate = require "AgentGate"
local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local ccoroutine = require "ccoroutine"
local util = require "util"
local timer = require "timer"

local local_modulename = ...
timer.register(local_modulename)

local event = {}


local client_list = {}
local count = 0
function event.OnRecvCall(proto, data)
	
end

function event.OnRecvCommon(proto, data)
	
end

function event.OnRegister()
	print("xxxxxxxxxxxxxxxxxxxx")
end

function event.OnRemoteRecvData(socket, proto, data)
	
end

function event.OnRemoteConnect(socket, ip)
client_list[socket] =socket
count = count + 1
end

function event.OnRemoteDisConnect(socket)
	client_list[socket] =nil
	count = count - 1
end

function event.framefunc()
	timer.addtimer(local_modulename, "framefunc", 1000)

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
