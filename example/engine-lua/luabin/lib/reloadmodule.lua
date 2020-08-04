local util = require "util"

local reloadmodule = {}

local package = package

function reloadmodule.reload(module_name)
	local old_module = package.loaded[module_name]

	package.loaded[module_name] = nil
	local ok,err = pcall(require, module_name)

	if not ok then
		package.loaded[module_name] = old_module
		print(string.format("hotfix fail, modulename=%s err=%s ", module_name, err))
		return false
	end

	local new_module = package.loaded[module_name]

	if type(old_module) == type({}) and type(new_module) ~= type({}) then
		package.loaded[module_name] = old_module
		print(debug.traceback(), "\n", string.format("hotfix fail, module change from table to not table. modulename=%s ", module_name))
		return false
	end

	if old_module == nil then
		package.loaded[module_name] = new_module
		print(string.format("hotfix ok, first require type=%s modulename=%s", type(new_module), module_name))
		return true
	end

	if type(old_module) == type(new_module) and type(old_module) == type({}) then
		package.loaded[module_name] = old_module
		for k, v in pairs(new_module) do
			old_module[k] = v
		end
		print(string.format("hotfix ok, module change from table to table modulename=%s", module_name))
		return true
	elseif type(old_module) ~= type({}) then
		old_module = new_module
		package.loaded[module_name] = old_module
		print(string.format("hotfix ok, module change from not table to %s modulename=%s", type(new_module), module_name))
		return true
	end

	package.loaded[module_name] = old_module
	print(debug.traceback(), "\n", string.format("hotfix fail, default change failed modulename=%s ", module_name))
	return false
end

function reloadmodule.reloadtest(module_name)
	local old_module = package.loaded[module_name]

	package.loaded[module_name] = nil
	local ok,err = pcall(require, module_name)

	if not ok then
		package.loaded[module_name] = old_module
		print(string.format("hotfix fail, modulename=%s err=%s ", module_name, err))
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
