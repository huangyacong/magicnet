local CoreNet = require "CoreNet"
local ccoroutine = require "ccoroutine"
local CoreTool = require "CoreTool"
local timer = require "timer"
local util = require "util"
require "class"

local ccorenet = {}
local sys_run = true
local sys_print = nil

local clientObj = {}
local svrObj = {}

local globalSingleObj = {}
local globalHotfixModuleName = {}

-- socket类型 
ccorenet.IpV4 = CoreNet.DOMAIN_INET
ccorenet.UnixLocal = CoreNet.DOMAIN_UNIX

-- iServer列表
ccorenet.IServerList = svrObj

-- IClient列表
ccorenet.IClientList = clientObj

-- 热更新的模块列表
ccorenet.HotfixModuleName = globalHotfixModuleName

-- 全局日志函数回调
function GlobalLogCallBack(logStr)
	sys_print(logStr)
end

-- 添加需要热更新的模块
function ccorenet.addHotfixModuleName(modulename)
	globalHotfixModuleName[tostring(modulename)] = true
end

-- 加入一个全局实例
function ccorenet.addGlobalObj(object, name)
	if globalSingleObj[name] then return globalSingleObj[name] end
	globalSingleObj[name] = object
	return globalSingleObj[name]
end

-- 获取一个全局实例
function ccorenet.getGlobalObj(name)
	return globalSingleObj[name]
end

-- 删除一个全局实例
function ccorenet.delGlobalObj(name)
	globalSingleObj[name] = nil
end

-- 判断操作系统 Linux,Windows,Unknow
function ccorenet.getOS()
	return CoreNet.GetOS()
end

-- 初始化
function ccorenet.init(cLogName, iMaxClientNum, bPrintLog2Screen)
	sys_print = print
	return CoreNet.Init(cLogName, iMaxClientNum, 16, bPrintLog2Screen)
end

-- 结束
function ccorenet.fin()
	clientObj, svrObj = {}, {}
	return CoreNet.Fin()
end

local net_event_fliter_svr = {
	[CoreNet.SOCKET_CONNECT] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnConnect(recvsocket, data)
								end,
	[CoreNet.SOCKET_DISCONNECT] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnDisConnect(recvsocket)
								end,
	[CoreNet.SOCKET_RECV_DATA] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnRecv(recvsocket, data)
								end,
}

local net_event_fliter_client = {
	[CoreNet.SOCKET_CONNECT] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnConnect(data)
								end,
	[CoreNet.SOCKET_CONNECT_FAILED] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnConnectFailed()
								end,
	[CoreNet.SOCKET_DISCONNECT] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnDisConnect()
								end,
	[CoreNet.SOCKET_RECV_DATA] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnRecv(data)
								end,
}

local function worker()
	local netevent, listenscoket, recvsocket, data = table.unpack(CoreNet.Read())
	if svrObj[listenscoket] then
		local tcpsocketobj = svrObj[listenscoket]
		local fliter = net_event_fliter_svr[netevent]
		if fliter then
			fliter(tcpsocketobj, netevent, listenscoket, recvsocket, data)
		else
			pcall(function ()  print(debug.traceback(), "\n", string.format("ccorenet.read svrObj not netevent=%s listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket)) end)
		end
	elseif clientObj[recvsocket] then
		local tcpsocketobj = clientObj[recvsocket]
		local fliter = net_event_fliter_client[netevent]
		if fliter then
			fliter(tcpsocketobj, netevent, listenscoket, recvsocket, data)
		else
			pcall(function ()  print(debug.traceback(), "\n", string.format("ccorenet.read clientObj not netevent=%s listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket)) end)
		end
	elseif CoreNet.SOCKET_TIMER == netevent then
		timer.timeout()
	else
		pcall(function ()  print(debug.traceback(), "\n", string.format("ccorenet.read event=%s not do listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket)) end)
	end
end

-- 运行
function ccorenet.start()
	local result, errMsg = pcall(function() 
		while sys_run == true do
			local ret, err = ccoroutine.resume(ccoroutine.co_create(worker))
			if not ret then print(debug.traceback(), "\n", string.format("ccoroutine.resume %s", err)) end
		end
	end)
	if not result then print(debug.traceback(), "\n", string.format("ccorenet.start %s", errMsg)) end
end

-- 停止
function ccorenet.exit()
	sys_run = false
end

-- 消息类型
ccorenet.PTYPE = {
	PTYPE_RESPONSE = 0,			-- 回应协程消息
	PTYPE_CALL = 1,				-- 协程消息
	PTYPE_REMOTE = 2,			-- 发送给远程目标数据类型
	PTYPE_COMMON = 3,			-- 普通类型
	PTYPE_REGISTER_KEY = 4,		-- 注册Key类型
	PTYPE_REGISTER = 5,			-- 注册类型
	PTYPE_PING = 6,				-- Ping类型
	PTYPE_SYSTEM = 7,			-- 系统类型
}

-- 生成token
function ccorenet.genToken(key, name)
	return CoreTool.MD5(key .. name.. "crtgame")
end

-- 打包
function ccorenet.pack(targetName, proto, data, PTYPE, session_id)
	session_id = session_id or 0
	return string.pack("zz>H>j", targetName, proto, PTYPE, session_id), data, PTYPE, session_id
end

-- 解包
function ccorenet.unpack(data)
	local targetName, proto, PTYPE, session_id, len = string.unpack("zz>H>j", data)
	return targetName, proto, string.sub(data, len, string.len(data)), PTYPE, session_id
end

-- 等待一个函数执行完毕
function ccorenet.waitEvent(event_id, f, ...)
	return ccoroutine.wait_event(event_id, f, ...)
end

-- 唤醒队列
function ccorenet.wakeUpQueue(queue_id, bPopHead, queueNum)
	return ccoroutine.wakeup_queue(queue_id, bPopHead, queueNum)
end

-- 加入队列
function ccorenet.waitQueue(queue_id)
	return ccoroutine.wait_queue(queue_id)
end

-- 挂起
function ccorenet.suspendTimeOut(timeout_millsec)
	return ccoroutine.wait_time_sleep(timeout_millsec)
end

-- 系统print函数钩子
function ccorenet.hookprint()

	if not sys_print then
		return
	end
	
	local hook_print = function(...)
		local cache = os.date("%Y-%m-%d %H:%M:%S")
		for k,v in ipairs(table.pack(...)) do
			cache = cache.." "..tostring(v)
		end
		GlobalLogCallBack(cache)
	end

	print = hook_print
end

-- 恢复系统print函数
function ccorenet.undolprint()
	if not sys_print then
		return
	end
	print = sys_print
end

-- 取消系统print函数
function ccorenet.cancelprint()
	if not sys_print then
		return
	end
	local hook_print = function(...) end
	print = hook_print
end

-- 获取统计数据
function ccorenet.statReport(timeDelay)
	return CoreNet.Report(timeDelay)
end

return util.ReadOnlyTable(ccorenet)
