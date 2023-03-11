local net_module = require "CCoreNet"
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

function iniconf.read(filename)
	local result = {}

	local file, err = io.open(filename, "rb")
	if nil == file then
		print(debug.traceback(), "\n", string.format("iniconf.read filename=%s failed %s", filename, err))
		return nil
	end

	repeat  
		local lines = file:read("l")
		if lines then
			lines = lines:gsub("%c", "")
			lines = lines:gsub("%s", "")
			lines = util.PathReplace(lines, net_module.getOS() == "Linux")

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
	return util.ReadOnlyTable(IniConfClass.new(result))
end

return util.ReadOnlyTable(iniconf)