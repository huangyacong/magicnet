local net_module = require "ccorenet"
local util = require "util"
require "class"

local IniConfClass = class()
function IniConfClass:ctor(fileData)
	self.fileData = fileData
end

function IniConfClass:GetFiledNumber(filed)
	return tonumber(self.fileData[filed])
end

function IniConfClass:GetFiledString(filed)
	return tostring(self.fileData[filed])
end

local iniconf = {}


local function StringPathFromWindowsToLinux(value)
	if net_module.getOS() == "Linux" then
		local path = ""
		for i=1,#value do
	        --获取当前下标字符串
			local tmp = string.sub(value,i,i)
	        --如果为'\\'则替换
			if tmp =='\\' then
				path = path..'/'
			else
				path = path..tmp
			end
		end
		return tostring(path)
	end
	return tostring(value)
end

function iniconf.read(filename)
	local result = {}

	local file = io.open(filename, "rb")
	if nil == file then
		print(debug.traceback(), "\n", string.format("iniconf.read filename=%s failed", filename))
		return nil
	end

	repeat  
		local lines = file:read("l")
		if lines then
			lines = lines:gsub("%c", "")
			lines = lines:gsub("%s", "")
			lines = StringPathFromWindowsToLinux(lines)

			if string.sub(lines, 1, 1) ~= "#" then
				local data = util.split(lines, "=")
				if #data == 2 then
					local key, value = table.unpack(data)
					result[key] = value
				end
			end
			
		end
	until (not lines)

	file:close()
	return util.ReadOnlyTable(IniConfClass.new(result));
end

return util.ReadOnlyTable(iniconf)