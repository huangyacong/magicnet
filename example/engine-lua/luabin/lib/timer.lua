local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local timer = {}

local timeoutObj = {}
local timer_register = {}
local max_run_timer_count = 1000

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
	local timeId = CoreNet.AddTimer((iMillSecTime <= 0) and 1 or iMillSecTime)
	if timeId == 0 then return nil end
	timeoutObj[timeId] = {modulename = timer_register[modulename], func = tostring(func_name_str), param = table.pack(...)}
	return timeId
end

-- 删除定时器
function timer.deltimer(timeId)
	CoreNet.DelTimer(timeId)
	timeoutObj[timeId] = nil
end

-- 执行定时器回调
function timer.timeout()
	local doNum = 0
	while true do
		local timeId = CoreNet.GetTimeOutId()
		local runFuncObj = timeoutObj[timeId]
		timeoutObj[timeId] = nil
		doNum = doNum + 1

		if not runFuncObj then
			break
		end

		local isOK, ret = pcall(function () package.loaded[runFuncObj.modulename][runFuncObj.func](table.unpack(runFuncObj.param)) end)
		if not isOK then pcall(function () print(debug.traceback(), "\n", string.format("modulename=%s func=%s", runFuncObj.modulename, runFuncObj.func), "\n", ret) end) end

		if doNum > max_run_timer_count then
			break
		end
	end
end

return util.ReadOnlyTable(timer)
