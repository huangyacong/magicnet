local util = {}

function util.print_table(root)
	if root == nil then
		print("[PRINT_TABLE] root is nil")
		return
	end
	if type(root) ~= type({}) then
		print("[PRINT_TABLE] root not table type")
		print(tostring(root))
		return
	end
	if not next(root) then
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
				table.insert(temp,"+" .. key .. _dump(v,space .. (next(t,k) and "|" or " " ).. string.rep(" ",#key),new_key))
			else
				table.insert(temp,"+" .. key .. " [" .. tostring(v).."]")
			end
		end
		return table.concat(temp,"\n"..space)
	end
	
	print("--------------begin print table--------------")
	print("            "..os.date("%Y-%m-%d %H:%M:%S"))
	pcall(function () return print(_dump(root, "","")) end)
	print("--------------end print table-----------------")
end

function util.print(root)
	util.print_table(root)
end

function util.ReadOnlyTable(t)
	local proxy = {}
	local mt = {
		__index = t,
		__newindex = function (t,k,v)
		error("attempt to update a read-only talbe",2)
	end
	}
	setmetatable(proxy,mt)
	return proxy
end

return util.ReadOnlyTable(util)
