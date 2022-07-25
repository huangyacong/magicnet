local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local IMsgClass = class()
function IMsgClass:ctor(iMsgID, strModuleName, strCallBackFuncName)
	self.iMsgID = iMsgID
	self.strModuleName = tostring(strModuleName)
	self.strCallBackFuncName = tostring(strCallBackFuncName)
end

function IMsgClass:getMsgID()
	return self.iMsgID
end

function IMsgClass:getCallBackMsg()
	return self.strModuleName, self.strCallBackFuncName
end

-- 消息订阅(同步的)
local IMsgSubscribeClass = class()
function IMsgSubscribeClass:ctor()
	self.oSubscribeMsgID = {}
	self.oSubscribeRoleID = {}
end

function IMsgSubscribeClass:unsubscribeAll()
	self.oSubscribeMsgID = {}
	self.oSubscribeRoleID = {}
end

function IMsgSubscribeClass:subscribe(iRoleID, iMsgID, strModuleName, strCallBackFuncName)

	require(strModuleName)
	local IMsgClassObj = IMsgClass.new(iMsgID, strModuleName, strCallBackFuncName)

	if not self.oSubscribeMsgID[iMsgID] then 
		self.oSubscribeMsgID[iMsgID] = {}
	end

	self.oSubscribeMsgID[iMsgID][iRoleID] = IMsgClassObj

	if not self.oSubscribeRoleID[iRoleID] then 
		self.oSubscribeRoleID[iRoleID] = {}
	end

	self.oSubscribeRoleID[iRoleID][iMsgID] = IMsgClassObj
end

function IMsgSubscribeClass:unsubscribe(iRoleID, iMsgID)

	if iMsgID then
		if self.oSubscribeMsgID[iMsgID] then
			self.oSubscribeMsgID[iMsgID][iRoleID] = nil
		end
		if self.oSubscribeRoleID[iRoleID] then
			self.oSubscribeRoleID[iRoleID][iMsgID] = nil
		end
	else
		if self.oSubscribeRoleID[iRoleID] then
			for iID,_ in pairs(self.oSubscribeRoleID[iRoleID]) do
				if self.oSubscribeMsgID[iID] then
					self.oSubscribeMsgID[iID][iRoleID] = nil
				end
			end
			self.oSubscribeRoleID[iRoleID] = nil
		end
	end

end

function IMsgSubscribeClass:runSubscribeByRoleID(iRoleID, iMsgID, ...)
	if not self.oSubscribeRoleID[iRoleID] or not self.oSubscribeRoleID[iRoleID][iMsgID] then 
		return
	end
	local param = table.pack(...)
	local timeCount = CoreTool.GetTickCount()
	local IMsgClassObj = self.oSubscribeRoleID[iRoleID][iMsgID]
	local strModuleName, strCallBackFuncName = IMsgClassObj:getCallBackMsg()
	local isOK, ret = xpcall(function () package.loaded[strModuleName][strCallBackFuncName](iRoleID, table.unpack(param)) end, debug.traceback)
	timeCount = CoreTool.GetTickCount() - timeCount
	if not isOK then 
		print("traceback error", "\n", string.format("runSubscribe modulename=%s func=%s", strModuleName, strCallBackFuncName), "\n", ret)
	end
	if timeCount > 16 then pcall(
		function () 
			print(string.format("runSubscribe timer Cost modulename=%s func=%s Time=%s", strModuleName, strCallBackFuncName, timeCount)) 
		end)
	end
end

function IMsgSubscribeClass:runSubscribeByMsgID(iMsgID, ...)
	if not self.oSubscribeMsgID[iMsgID] then 
		return
	end
	local result = {}
	for iRoleID,IMsgClassObj in pairs(self.oSubscribeMsgID[iMsgID]) do
		table.insert(result, {iRoleID, IMsgClassObj})
	end
	local param = table.pack(...)
	for _,value in ipairs(result) do
		local iRoleID, IMsgClassObj = table.unpack(value)
		local timeCount = CoreTool.GetTickCount()
		local strModuleName, strCallBackFuncName = IMsgClassObj:getCallBackMsg()
		local isOK, ret = xpcall(function () package.loaded[strModuleName][strCallBackFuncName](iRoleID, table.unpack(param)) end, debug.traceback)
		timeCount = CoreTool.GetTickCount() - timeCount
		if not isOK then 
			print("traceback error", "\n", string.format("runSubscribe modulename=%s func=%s", strModuleName, strCallBackFuncName), "\n", ret)
		end
		if timeCount > 16 then pcall(
			function () 
				print(string.format("runSubscribe timer Cost modulename=%s func=%s Time=%s", strModuleName, strCallBackFuncName, timeCount)) 
			end)
		end
	end
end

local MsgSubscribe = {}

function MsgSubscribe.new()
	return IMsgSubscribeClass.new()
end

return MsgSubscribe