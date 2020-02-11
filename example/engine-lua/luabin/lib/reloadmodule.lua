local util = require "util"

local reloadmodule = {}

function reloadmodule.reload(module_name)
	local old_module = package.loaded[module_name] or {}

	package.loaded[module_name] = nil
	local ok,err = pcall(require, module_name)

	if not ok then
		print(string.format("hotfix fail, err=%s modulename=%s", err, module_name))
		package.loaded[module_name] = old_module
		return false
	end

	local new_module = package.loaded[module_name]
	for k, v in pairs(new_module or {}) do
		old_module[k] = v
	end

	package.loaded[module_name] = old_module
	print(string.format("hotfix ok, modulename=%s", module_name))
	return true
end

function reloadmodule.reloadtest(module_name)
	local old_module = package.loaded[module_name]

	package.loaded[module_name] = nil
	local ok,err = pcall(require, module_name)

	if not ok then
		print(string.format("hotfix test fail, err=%s modulename=%s", err, module_name))
		package.loaded[module_name] = old_module
		return false
	end

	package.loaded[module_name] = old_module
	print(string.format("hotfix test ok, modulename=%s", module_name))
	return true
end

function reloadmodule.reloadlist(module_name_list)
	for _, v in ipairs(module_name_list) do
		if not reloadmodule.reloadtest(v) then
			return false
		end
	end
	for _, v in ipairs(module_name_list) do
		reloadmodule.reload(v) 
	end
	return true
end

return util.ReadOnlyTable(reloadmodule)
