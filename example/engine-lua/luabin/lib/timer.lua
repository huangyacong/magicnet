local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local timer = {}

local timercount = 0
local timeoutObj = {}
local timer_register = {}
local max_run_timer_count = 1000
local package = package
local table = table

-- 定时器注册事件,这个事件，最好添加在每个文件的开头
--[[
		示例
		local local_modulename = ...
		timer.register(local_modulename)
]]
function timer.register(modulename)
	local add_modulename = tostring(modulename)
	timer_register[add_modulename] = add_modulename
	return true
end

-- 添加定时器
function timer.addtimer(modulename, func_name_str, iMillSecTime, ...)
	if not timer_register[modulename] then
		print(debug.traceback(), "\n", string.format("timer.addtimer modulename=%s is not register timer id", modulename))
		return nil
	end
	local timeId = CoreTool.AddTimer((iMillSecTime <= 0) and 1 or iMillSecTime)
	if timeId == 0 then return nil end
	if not timeoutObj[timeId] then timercount = timercount + 1 end
	timeoutObj[timeId] = {modulename = timer_register[modulename], func = tostring(func_name_str), param = table.pack(...)}
	return timeId
end

-- 删除定时器
function timer.deltimer(timeId)
	if timeoutObj[timeId] then timercount = timercount - 1 end
	CoreTool.DelTimer(timeId)
	timeoutObj[timeId] = nil
end

-- 定时器个数
function timer.gettimercount()
	return timercount
end

-- 执行定时器回调
function timer.timeout()
	local doNum = 0
	while true do
		local timeId = CoreTool.GetTimeOutId()
		local runFuncObj = timeoutObj[timeId]
		timeoutObj[timeId] = nil
		doNum = doNum + 1

		if not runFuncObj then
			break
		end

		timercount = timercount - 1
		local timeCount = CoreTool.GetTickCount()

		local isOK, ret = xpcall(function () package.loaded[runFuncObj.modulename][runFuncObj.func](table.unpack(runFuncObj.param)) end, debug.traceback)
		if not isOK then pcall(function () print("traceback error", "\n", string.format("modulename=%s func=%s", runFuncObj.modulename, runFuncObj.func), "\n", ret) end) end

		timeCount = CoreTool.GetTickCount() - timeCount
    	if timeCount > 16 then pcall(
    		function () 
    			print(string.format("timer Cost modulename=%s func=%s Time=%s", runFuncObj.modulename, runFuncObj.func, timeCount)) 
    		end)
    	end

		if doNum > max_run_timer_count then
			break
		end
	end
	return doNum
end

return util.ReadOnlyTable(timer)
