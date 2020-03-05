local msgpack = require "msgpack53"
local ccorenet = require "ccorenet"
local reloadmodule = require "reloadmodule"
local ccoroutine = require "ccoroutine"
local util = require "util"
local CoreNet = require "CoreNet"
local CoreTool = require "CoreTool"
local timer = require "timer"

local local_modulename = ...
timer.register(local_modulename)

local svr_event = {}

local count = 0
function svr_event.OnSPlayerConnect(IServerClassObj, socket, ip)
	print("OnSPlayerConnect----", socket, ip)
	count = count + 1
	ccorenet.getGlobalObj("serverObj"):SendData(socket, 123, ip)
end

function svr_event.OnSPlayerDisConnect(IServerClassObj, socket)
	print("OnSPlayerDisConnect", socket)
	count = count - 1
end

function svr_event.OnSPlayerRecv(IServerClassObj, socket, proto, ret)
	print("OnSPlayerRecv", socket, proto, ret)
end

function svr_event.framefunc()
	local report = ccorenet.statReport((ccorenet.getOS() ~= "Linux") and (30 * 1000) or 0)
	if report then
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount = table.unpack(report)
		print(string.format("statreport count=%s sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s timerCount=%s %s %s pool=%s", 
			count, sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine(), ccoroutine.count_coroutine_pool()))
	end
end

return svr_event