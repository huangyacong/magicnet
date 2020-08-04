local util = require "util"
local table_insert = table.insert

local math = math
math.randomseed(os.time())

local randomutil = {}

function randomutil.random()
	return math.random()
end

function randomutil.random_int(lower, upper)
	return math.floor(lower + (upper - lower) * math.random())
end

function randomutil.random_array(table)
	local index = math.random(1, #table)
	return table[math.floor(index)]
end

return util.ReadOnlyTable(randomutil)
