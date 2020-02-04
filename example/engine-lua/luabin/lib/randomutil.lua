local util = require "util"

local randomutil = {}

function randomutil.init_seed()
	math.randomseed(os.time())
end

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
