local CoreNet = require "CoreNet"
local ccoroutine = require "ccoroutine"
local CoreTool = require "CoreTool"
local util = require "util"
require "class"

local ccorenet = {}
local sys_run = true
local sys_print = nil

local timeoutObj = {}
local frameObj = {}
local clientObj = {}
local svrObj = {}

local globalSingleObj = {}

-- socket类型 
ccorenet.IpV4 = CoreNet.DOMAIN_INET
ccorenet.UnixLocal = CoreNet.DOMAIN_UNIX

-- iServer列表
ccorenet.IServerList = svrObj

-- IClient列表
ccorenet.IClientList = clientObj

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
function ccorenet.init(cLogName, iMaxClientNum, iTimerCnt, bPrintLog2Screen)
	sys_print = print
	ccorenet.setframefunc(ccorenet, "framefunc")
	return CoreNet.Init(cLogName, iMaxClientNum, iTimerCnt, bPrintLog2Screen)
end

-- 结束
function ccorenet.fin()
	clientObj, svrObj = {}, {}
	return CoreNet.Fin()
end

-- 默认的帧回调
function ccorenet.framefunc()
	print("please set time func")
end

-- 设置帧回调数据
function ccorenet.setframefunc(module_name, frame_func_name_str)
	frameObj.modulename = module_name
	frameObj.frame_func_name_str = frame_func_name_str
end

-- 执行帧的函数
local function frame_run()
	local isOK, ret = pcall(function () frameObj.modulename[frameObj.frame_func_name_str]() end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
end

-- 添加定时器
function ccorenet.addtimer(modulename, func_name_str, iMillSecTime, ...)
	local timeId = CoreNet.AddTimer(iMillSecTime)
	if timeId == 0 then return nil end
	timeoutObj[timeId] = {modulename = modulename, func = func_name_str, param = table.pack(...)}
	return timeId
end

-- 删除定时器
function ccorenet.deltimer(timeId)
	CoreNet.DelTimer(timeId)
	timeoutObj[timeId] = nil
end

-- 执行定时器回调
local function timeout_run()
	local timeId = CoreNet.GetTimeOutId()
	local runFuncObj = timeoutObj[timeId]
	timeoutObj[timeId] = nil
	if runFuncObj then
		local isOK, ret = pcall(function () runFuncObj.modulename[runFuncObj.func](table.unpack(runFuncObj.param)) end)
		if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
	end
end

local function worker()
	local netevent, listenscoket, recvsocket, data = table.unpack(CoreNet.Read())
	if svrObj[listenscoket] then
		local tcpsocketobj = svrObj[listenscoket]
		if netevent == CoreNet.SOCKET_RECV_DATA then
			local isOK, ret = pcall(function () tcpsocketobj:OnRecv(recvsocket, data) end)
			if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
		elseif netevent == CoreNet.SOCKET_CONNECT then
			local isOK, ret = pcall(function () tcpsocketobj:OnConnect(recvsocket, data) end)
			if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
		elseif netevent == CoreNet.SOCKET_DISCONNECT then
			local isOK, ret = pcall(function () tcpsocketobj:OnDisConnect(recvsocket) end)
			if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
		else
			pcall(function ()  print(string.format("ccorenet.read svrObj not netevent=%s listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket)) end)
		end
	elseif clientObj[recvsocket] then
		local tcpsocketobj = clientObj[recvsocket]
		if netevent == CoreNet.SOCKET_RECV_DATA then
			local isOK, ret = pcall(function () tcpsocketobj:OnRecv(data) end)
			if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
		elseif netevent == CoreNet.SOCKET_CONNECT then
			local isOK, ret = pcall(function () tcpsocketobj:OnConnect(data) end)
			if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
		elseif netevent == CoreNet.SOCKET_CONNECT_FAILED then
			local isOK, ret = pcall(function () tcpsocketobj:OnConnectFailed() end)
			if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
		elseif netevent == CoreNet.SOCKET_DISCONNECT then
			local isOK, ret = pcall(function () tcpsocketobj:OnDisConnect() end)
			if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
		else
			pcall(function ()  print(string.format("ccorenet.read clientObj not netevent=%s listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket)) end)
		end
	elseif CoreNet.SOCKET_TIMER == netevent then
		frame_run()
		timeout_run()
	else
		pcall(function ()  print(string.format("ccorenet.read event=%s not do listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket)) end)
	end
end

-- 运行
function ccorenet.start()
	local isOK, errMsg = pcall(function ()
			while sys_run == true do
				local ret, err = ccoroutine.resume(ccoroutine.co_create(worker))
				if not ret then pcall(function () print(debug.traceback(), "\n", err) end) end
			end
		end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", errMsg) end) end
end

-- 停止
function ccorenet.exit()
	sys_run = false
end

-- 消息类型
ccorenet.PTYPE = {
	PTYPE_COMMON = 0,	-- 普通类型
	PTYPE_CALL = 1,		-- 协程消息
	PTYPE_RESPONSE = 2,	-- 回应协程消息
}

-- 打包
function ccorenet.pack(proto, data, PTYPE, session_id)
	session_id = session_id or 0
	return string.pack("s16>H>j", proto, PTYPE, session_id), data, PTYPE, session_id
end

-- 解包
function ccorenet.unpack(data)
	local proto, PTYPE, session_id, len = string.unpack("s16>H>j", data)
	return proto, string.sub(data, len, string.len(data)), PTYPE, session_id
end

-- 系统print函数钩子
function ccorenet.hookprint()

	if not sys_print then
		return
	end
	
	local hook_print = function(...)
		local cache = ""
		for k,v in ipairs(table.pack(...)) do
			cache = cache.." "..tostring(v)
		end
		CoreNet.HookPrint(cache)
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
function ccorenet.statreport()
	return CoreNet.Report()
end

return util.ReadOnlyTable(ccorenet)
