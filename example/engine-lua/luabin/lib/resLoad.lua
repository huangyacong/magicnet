local net_module = require "CCoreNet"
local msgpack = require "msgpack53"
local util = require "util"

local mainPath = ".\\res\\"

local resLoad = {}

function resLoad.LoadRes(filename)
	filename = filename:lower()
	local file, err = io.open(util.PathReplace(mainPath .. filename, net_module.getOS() == "Linux"), "rb")

	if nil == file then
		assert(false, string.format("LoadRes filename=%s failed %s", filename, err))
		return nil
	end

	local data = file:read("a")
	file:close()

	local resData = msgpack.unpack(data)
	return util.copytable(resData, true)
end

return resLoad
