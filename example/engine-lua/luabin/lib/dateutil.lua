local util = require "util"

local dateutil = {}

-- 获取当前时间,单位秒
function dateutil.now_time()
	return math.floor(os.time())
end

-- 增加小时
function dateutil.add_hour(hours)
	return (math.floor(hours)*3600)
end

-- 增加分钟
function dateutil.add_min(mins)
	return (math.floor(mins)*60)
end

-- 格式化时间字符串
function dateutil.format_date(mytime)
	return os.date("%Y-%m-%d %H:%M:%S", mytime)
end

-- 格式化时间字符串
function dateutil.format_day(time)
	return os.date("%Y-%m-%d", time)
end

--创建一个时间表
function dateutil.format_time_table(time)
	return os.date("*t",time)
end

--将时间格式化秒 %Y-%m-%d %H:%M:%S
function dateutil.format_second(time)
    local a = util.split(time, " ")
    local b = util.split(a[1], "-")
    local c = util.split(a[2], ":")
    return os.time({year=b[1],month=b[2],day=b[3], hour=c[1], min=c[2], sec=c[3]})
end

--获取一个月有多少天
function dateutil.get_days_of_month(year, month)
	local year = tonumber(year)
	local month = tonumber(month)
	
	if month == 1 or month == 3 or month == 5
		or month == 7 or month == 8 or month == 10
		or month == 12 then
		return 31
	end

	if month == 2 then
		if (year%4==0 and year%100~=0) or (year%400==0) then
			return 29
		else
			return 28
		end
	end

	return 30
end

-- 获取基于当前指定时间点的时间戳
function dateutil.get_time_stamp(sec, min, hour, day, month, year)
	local time_tbl = os.date("*t", os.time())
	local time_struct = {}
	
	if sec then
		time_struct.sec = tonumber(sec)
	else
		time_struct.sec = time_tbl.sec
	end
	
	if min then
		time_struct.min = tonumber(min)
	else
		time_struct.min = time_tbl.min
	end

	if hour then
		time_struct.hour = tonumber(hour)
	else
		time_struct.hour = time_tbl.hour
	end

	if day then
		time_struct.day = tonumber(day)
	else
		time_struct.day = time_tbl.day
	end

	if month then
		time_struct.month = tonumber(month)
	else
		time_struct.month = time_tbl.month
	end

	if year then
		time_struct.year = tonumber(year)
	else
		time_struct.year = time_tbl.year
	end

	return os.time(time_struct)
end

return util.ReadOnlyTable(dateutil)
