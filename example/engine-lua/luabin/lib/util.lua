local CoreTool = require "CoreTool"

local util = {}

local printTableConf = true
local table = table
local string = string

function util.setPrintTable(bPrint)
	printTableConf = bPrint
end

function util.print_table(root)

	if not printTableConf then
		return
	end

	if root == nil then
		print("[PRINT_TABLE] root is nil")
		return
	end
	if type(root) ~= type({}) then
		print("[PRINT_TABLE] root not table type")
		print(tostring(root))
		return
	end
	
	local cache = {  [root] = "." }
	local function _dump(t,space,name)
		local temp = {}
		for k,v in pairs(t) do
			local key = tostring(k)
			if cache[v] then
				table.insert(temp,"+" .. key .. " {" .. cache[v].."}")
			elseif type(v) == "table" then
				local new_key = name .. "." .. key
				cache[v] = new_key
				table.insert(temp,"+" .. key .. _dump(v,space .. (rawget(t,k) and "|" or " " ).. string.rep(" ",#key),new_key))
			else
				table.insert(temp,"+" .. key .. " [" .. tostring(v).."]")
			end
		end
		return table.concat(temp,"\n"..space)
	end
	
	print("--------------begin print table--------------")
	print("            "..os.date("%Y-%m-%d %H:%M:%S"))
	pcall(function () return print("\n" .. _dump(root, "","")) end)
	print("--------------end print table-----------------")
end

function util.print(root)
	util.print_table(root)
end

local function copy_table(data, bReadOnly)
	local tab = {}
	for k, v in pairs(data or {}) do
		if type(v) ~= "table" then
			tab[k] = v
		else
			tab[k] = copy_table(v, bReadOnly)
		end
	end
	if bReadOnly then
		return util.ReadOnlyTable(tab) 
	end
	return tab
end

-- table拷贝
function util.copytable(data, bReadOnly)
	return copy_table(data, bReadOnly)
end

function util.split(str, pat)
   	local t = {}  
   	local fpat = "(.-)" .. ((pat == "") and "%s" or pat)
   	local last_end = 1
   	local s, e, cap = str:find(fpat, 1)
	while s do
	 	if s ~= 1 or cap ~= "" then
	     	table.insert(t,cap)
	    end
	    last_end = e+1
	    s, e, cap = str:find(fpat, last_end)
	end

	if last_end <= #str then
	    cap = str:sub(last_end)
	    table.insert(t, cap)
	end

	return t
end

-- windows和linux的路径相互替换
function util.PathReplace(str, bLinuxOS)

	local function replace(value, patSrc, patTarget)
		local path = ""
		for i=1, #value do
			local tmp = string.sub(value,i,i)
			if tmp == patSrc then
				path = path .. patTarget
			else
				path = path .. tmp
			end
		end
		return tostring(path)
	end

	local patSrc, patTarget = '/', '\\'
	if bLinuxOS then
		patSrc, patTarget = '\\', '/'
	end

	return replace(str, patSrc, patTarget)
end

function util.get_arg(num)
	return arg[num]
end

function util.ReadOnlyTable(t)
	local proxy = {}
	local mt = {
		__index = t,
		__len = function (a) return #t end,
		__pairs = function (a) return pairs(t) end,
		__ipairs = function (a) return ipairs(t) end,
		__newindex = function (a, k, v) error("attempt to update a read-only talbe",2) end
	}
	setmetatable(proxy,mt)
	return proxy
end

local keyWords = {["__bUpdate"] = true, ["__iSessionId"] = true}

function util.AddTableAutoUpdateMsg(t, bchange, keyWordsArray)
	for k, v in pairs(t) do
		assert(not keyWords[k], string.format("Class Has Key=%s, this can only use in system!", k))
	end
	for _, k in ipairs(keyWordsArray or {}) do
		--assert(keyWords[k] == nil, string.format("AddTableAutoUpdateMsg keyWords=%s more define!", k))
		keyWords[k] = true
	end
	
	t.__bUpdate = bchange and true or false
	t.__iSessionId = CoreTool.SysSessionId()

	local proxy = {}
	local mt = {
		__index = t,
		__len = function (a) return #t end,
		__pairs = function(a) -- 把关键字过滤掉
			return function(a, k)
					local v
					repeat
						k, v = next(a, k)
					until k == nil or not keyWords[k]
					return k, v
			end, t, nil
		end,
		__ipairs = function (a) return ipairs(t) end,
		__newindex = function (a, k, v)
			t[k] = v
			if not keyWords[k] then
				t.__bUpdate = true
				t.__iSessionId = CoreTool.SysSessionId()
			end
		end
	}
	setmetatable(proxy,mt)
	return proxy
end

local function tableHasAutoUpdateMsg(checkTable)
	assert(checkTable.__bUpdate ~= nil)
	if checkTable.__bUpdate == true then
		return true
	end
	for k, v in pairs(checkTable) do
		if type(k) == type({}) then
			if tableHasAutoUpdateMsg(k) then
				return true
			end
		end
		if type(v) == type({}) then
			if tableHasAutoUpdateMsg(v) then
				return true
			end
		end
	end
	return false
end

function util.TableHasAutoUpdateMsg(checkTable)
	return tableHasAutoUpdateMsg(checkTable)
end

return util.ReadOnlyTable(util)
