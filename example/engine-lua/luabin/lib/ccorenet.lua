local CoreNetAgent = require "CoreNetAgent"
local ccoroutine = require "ccoroutine"
local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local timer = require "timer"
local util = require "util"
require "class"

local table = table

local ccorenet = {}
local sys_run = true
local sys_print = nil

local clientObj = {}
local svrObj = {}

local globalSingleObj = {}
local globalHotfixModuleName = {}

-- socket类型 
ccorenet.IpV4 = CoreNet.DOMAIN_INET
ccorenet.IpV6 = CoreNet.DOMAIN_INET6
ccorenet.UnixLocal = CoreNet.DOMAIN_UNIX

-- iServer列表
ccorenet.IServerList = svrObj

-- IClient列表
ccorenet.IClientList = clientObj

-- 热更新的模块列表
ccorenet.HotfixModuleName = globalHotfixModuleName

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

local net_event_id = {
	[CoreNet.SOCKET_CONNECT] = CoreNet.SOCKET_CONNECT,
	[CoreNet.SOCKET_CONNECT_FAILED] = CoreNet.SOCKET_CONNECT_FAILED,
	[CoreNet.SOCKET_DISCONNECT] = CoreNet.SOCKET_DISCONNECT,
	[CoreNet.SOCKET_RECV_DATA] = CoreNet.SOCKET_RECV_DATA,
	[CoreNet.SOCKET_TIMER] = CoreNet.SOCKET_TIMER,
}

local function worker()
	local netevent, listenscoket, recvsocket, data = table.unpack(CoreNet.Read())
	
	if net_event_id[netevent] then
		if svrObj[listenscoket] then
			local tcpsocketobj = svrObj[listenscoket]
			local fliter = net_event_fliter_svr[netevent]
			if fliter then
				fliter(tcpsocketobj, netevent, listenscoket, recvsocket, data)
			else
				print(debug.traceback(), "\n", string.format("ccorenet.read svrObj not netevent=%s listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket))
			end
		elseif clientObj[recvsocket] then
			local tcpsocketobj = clientObj[recvsocket]
			local fliter = net_event_fliter_client[netevent]
			if fliter then
				fliter(tcpsocketobj, netevent, listenscoket, recvsocket, data)
			else
				print(debug.traceback(), "\n", string.format("ccorenet.read clientObj not netevent=%s listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket))
			end
		elseif CoreNet.SOCKET_TIMER == netevent then
			timer.timeout()
		else
			print(debug.traceback(), "\n", string.format("ccorenet.read event=%s not find listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket))
		end
	else
		print(debug.traceback(), "\n", string.format("ccorenet.read event=%s not do listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket))
	end
end

-- 运行
function ccorenet.start()
	while sys_run == true do
		local ret, co = pcall(function() return ccoroutine.co_create(worker) end)
		if not ret then pcall(function() sys_print(debug.traceback(), "\n", string.format("ccoroutine.co_create %s", co)) end) end
		if co then ccoroutine.resume(co) end
	end
	sys_print(string.format("exit.... %s", sys_run))
end

-- 停止
function ccorenet.exit()
	sys_run = false
end

-- 生成token
function ccorenet.genToken(key, name)
	return CoreNetAgent.GenRegToken(key, name)
end

-- 打包
function ccorenet.netPack(srcName, targetName, proto, PTYPE, session_id, data)
	session_id = session_id or 0
	local header = CoreNetAgent.NetPack(srcName, targetName, PTYPE, session_id, proto)
	return header, data
end

-- 解包
function ccorenet.netUnPack(data)
	local srcName, targetName, PTYPE, session_id, proto, recvData = CoreNetAgent.NetUnPack(data)
	return srcName, targetName, PTYPE, session_id, proto, recvData
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
local logLock = false
local serviceName = ""
local LogFunctionCallBack = nil

local function hook_print(...)
	
	local cacheArray = {}
	table.insert(cacheArray, "[" .. serviceName .. "]")
	for k,v in ipairs(table.pack(...)) do 
		table.insert(cacheArray, " ") 
		if type(v) == type("") then
			table.insert(cacheArray, v) 
		else
			table.insert(cacheArray, tostring(v)) 
		end
	end
	local cache = table.concat(cacheArray)

	if logLock then
		sys_print(cache)
		return
	end

	logLock = true
	local result, errMsg = pcall(function() LogFunctionCallBack(cache) end)
	if not result then sys_print(debug.traceback(), "\n", string.format("GlobalLogCallBack %s", errMsg)) end
	logLock = false
end

function ccorenet.hookprint(nameStr, OnLogFunctionCallBack)

	if not sys_print then
		return
	end

	serviceName = nameStr
	LogFunctionCallBack = OnLogFunctionCallBack
	
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

-- 系统打印
function ccorenet.sys_print(...)
	sys_print(...)
end

return util.ReadOnlyTable(ccorenet)
