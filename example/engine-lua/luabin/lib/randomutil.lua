local util = require "util"
local dateutil = require "dateutil"
local table_insert = table.insert

local math = math
math.randomseed(dateutil.now_time())

local randomutil = {}

function randomutil.random()
	return math.random()
end

function randomutil.random_int(lower, upper)
	return math.random(lower, upper)
end

function randomutil.random_array(table)
	local index = math.random(1, #table)
	return table[index]
end

return util.ReadOnlyTable(randomutil)
