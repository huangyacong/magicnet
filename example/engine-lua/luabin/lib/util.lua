local CoreTool = require "CoreTool"

local util = {}

local printTableConf = true
local table = table
local string = string
local Next = next

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
		assert(type(k) ~= "table")
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

local __metatableName = "automsg"

local function __next(tb, ...)
	local metatable = getmetatable(tb)

	if not metatable or metatable ~= __metatableName then
		return Next(tb, ...)
	end

	for key, value in pairs(tb) do
		return value
	end

	return nil
end
next = __next

function util.ReadOnlyTable(t)
	local proxy = {}
	local mt = {
		__metatable = __metatableName,
		__index = t,
		__len = function (a) return #t end,
		__pairs = function (a) return pairs(t) end,
		__ipairs = function (a) return ipairs(t) end,
		__newindex = function (a, k, v) error("attempt to update a read-only talbe",2) end
	}
	setmetatable(proxy,mt)
	return proxy
end

local oAutoClass = setmetatable({}, { __mode = "k" })
local oGlobalIgnoreKeys = {["oInstance"] = true, ["oModuleInterface"] = true}
local keyWords = {["__bUpdate"] = true, ["__iSessionId"] = true, ["__oIgnoreKeys"] = true, ["__fModifyCallBack"] = true}

local __meta = {__metatable = __metatableName}

function __meta:__index(key)
	local tbl = oAutoClass[self]
	return tbl[key]
end

function __meta:__len()
	local tbl = oAutoClass[self]
	return #tbl
end

function __meta:__pairs()
	return function(tbl, key)
		local value
		repeat
			key, value = Next(tbl, key)
		until key == nil or not keyWords[key]
		return key, value
	end, oAutoClass[self], nil
end

function __meta:__ipairs() 
	local tbl = oAutoClass[self]
	return ipairs(tbl) 
end

function __meta:__newindex(key, value)
	local tbl = oAutoClass[self]
	tbl[key] = value
	if not keyWords[key] and not tbl.__oIgnoreKeys[key] then
		local iSessionId = tbl.__iSessionId

		tbl.__bUpdate = true
		tbl.__iSessionId = CoreTool.SysSessionId()
		
		if iSessionId >= tbl.__iSessionId then 
			tbl.__iSessionId = iSessionId + 1 
		end

		if tbl.__fModifyCallBack then
			local fModifyCallBack, argv = table.unpack(tbl.__fModifyCallBack)
			fModifyCallBack(table.unpack(argv))
		end
	end
end

function util.GetAutoClass()
	return oAutoClass
end

function util.AddTableAutoUpdateMsg(tbl, bchange, IgnoreKeys, fModifyCallBack, ...)

	for k, v in pairs(tbl) do
		assert(not keyWords[k], string.format("Class Has Key=%s, this can only use in system!", k))
	end
	
	tbl.__bUpdate = bchange and true or false
	tbl.__iSessionId = CoreTool.SysSessionId()
	tbl.__oIgnoreKeys = {}

	local argvs = {}
	for _,argv in ipairs(table.pack(...)) do 
		table.insert(argvs, argv)
	end
	tbl.__fModifyCallBack = fModifyCallBack and {fModifyCallBack, argvs} or nil

	for _, k in ipairs(IgnoreKeys or {}) do
		tbl.__oIgnoreKeys[tostring(k)] = true
	end

	local proxy = {}
	oAutoClass[proxy] = tbl
	setmetatable(proxy, __meta)
	return proxy
end

local function __NeedCheckAutoUpdateMsg(checkTable, key, value)

	if oGlobalIgnoreKeys[key] or type(value) ~= type({}) then
		return false
	end

	if checkTable.__oIgnoreKeys ~= nil and checkTable.__oIgnoreKeys[key] ~= nil then
		return false
	end

	return true
end

function util.TableHasAutoUpdateMsg(checkTable)

	if checkTable.__bUpdate ~= nil and checkTable.__bUpdate == true then
		return true
	end

	for k, v in pairs(checkTable) do
		if __NeedCheckAutoUpdateMsg(checkTable, k, v) then
			if util.TableHasAutoUpdateMsg(v) then
				return true
			end
		end
	end
	
	return false
end

function util.SetTableAutoUpdateMsg(SetTable, flag)

	if SetTable.__bUpdate ~= nil and type(flag) == type(true) then
		SetTable.__bUpdate = flag
	end

	for k, v in pairs(SetTable) do
		if __NeedCheckAutoUpdateMsg(SetTable, k, v) then
			util.SetTableAutoUpdateMsg(v, flag)
		end
	end
	
end

function util.GetTableSessionId(GetTable)

	local __iSessionId = 0

	if GetTable.__iSessionId ~= nil then
		__iSessionId = GetTable.__iSessionId
	end

	local function GetMaxSessionId(GetSubTable)
		for k, v in pairs(GetSubTable) do
			if __NeedCheckAutoUpdateMsg(GetSubTable, k, v) then
				if v.__iSessionId ~= nil and v.__iSessionId > __iSessionId then
					__iSessionId = v.__iSessionId
				end
				GetMaxSessionId(v)				
			end
		end
	end

	GetMaxSessionId(GetTable)

	assert(__iSessionId ~= nil)
	return __iSessionId
end

-- 慎用，可能有性能问题
function util.TableLength(Table)
	local Count = 0
	for k, v in pairs(Table) do
		Count = Count + 1
	end
	return Count
end

-- 数组是否包含元素
function util.Contains(array, val)
	for index, value in ipairs(array) do
		if value == val then
			return true
		end
	end
	return false
end

-- 打印错误
function util.error(msg, ...)
	print(string.format(msg, ...), "\n", debug.traceback())
end

-- 字符串是否为nil或者空
function util.isempty(s)
	return s == nil or s == ''
end

-- 倒叙
function util.reverseTable(tab)
	local tmp = {}
	for i = #tab, 1, -1 do
		table.insert(tmp, tab[i])
 	end
	return tmp
end

return util.ReadOnlyTable(util)
