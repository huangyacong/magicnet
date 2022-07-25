local CoreTool = require "CoreTool"
local util = require "util"

local dateutil = {}

local math = math
local timeos = os.time

local function now_time()
	return math.floor(timeos()) + CoreTool.GetTimeOffSet()
end

local function sys_os_time(...)
	local pram = table.pack(...)
	if #pram <= 0 then
		return now_time()
	else
		return math.floor(timeos(...))
	end
end
os.time = sys_os_time

-- 获取当前时间,单位秒
function dateutil.now_time()
	return now_time()
end

-- 修改服务器时间
function dateutil.ModifyTimeOffset(iTimeOffset)
	local iMaxTime, iModifyMaxTime = 20 * 365 * 24 * 3600, 5 * 365 * 24 * 3600
	if iTimeOffset >= iModifyMaxTime or iTimeOffset <= (iModifyMaxTime * -1) then
		return false
	end 
	if CoreTool.GetTimeOffSet() >= iMaxTime or CoreTool.GetTimeOffSet() <= (iMaxTime * -1) then
		return false
	end
	return CoreTool.ModifyTimeOffSet(iTimeOffset)
end

-- 恢复服务器时间
function dateutil.ResetTimeOffset()
	return CoreTool.ResetTimeOffSet()
end

-- 一共修改的时间，单位秒
function dateutil.GetTimeOffSet()
	return CoreTool.GetTimeOffSet()
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
	assert(time)
	return os.date("*t",time)
end

--将时间格式化秒 %Y-%m-%d %H:%M:%S
function dateutil.format_second(time)
    local a = util.split(time, " ")
    local b = util.split(a[1], "-")
    local c = util.split(a[2], ":")
    return os.time({year=b[1],month=b[2],day=b[3], hour=c[1], min=c[2], sec=c[3]})
end

--当天0点的时间
function dateutil.today_zero_seconds(time)
	if not time then
		time = dateutil.format_time_table(now_time())
	end
	return math.floor(os.time({
		year=time.year,
		month=time.month,
		day=time.day,
		hour = 0,
		min = 0,
		sec = 0
	}))
end

--当天过了多少秒
function dateutil.seconds_in_today(time)
	if not time then
		time = dateutil.format_time_table(now_time())
	end
	local todayTime = os.time({
		year=time.year,
		month=time.month,
		day=time.day,
		hour = 0,
		min = 0,
		sec = 0
	})
	return math.floor(os.time(time)) - math.floor(todayTime)
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
	local time_tbl = os.date("*t", dateutil.now_time())
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

-- 获取星期几(星期日是1)
function dateutil.get_weekday(time)
	return dateutil.format_time_table(time).wday
end

-- 获取上一个星期几 几时几分几秒的时间(星期日是1)
function dateutil.get_last_weekday_time(cur_time, weekday, hour, min, sec)
	local t = os.date("*t", cur_time)
	t.day = t.day - t.wday + weekday
	t.hour = hour
	t.min = min
	t.sec = sec
	if os.time(t) > cur_time then
		t.day = t.day - 7
	end 
	return math.floor(os.time(t))
end 

-- 获取下一个星期几 几时几分几秒的时间(星期日是1)
function dateutil.get_next_weekday_time(cur_time, weekday, hour, min, sec)
	local t = os.date("*t", cur_time)
	t.day = t.day - t.wday + weekday
	t.hour = hour
	t.min = min
	t.sec = sec
	if os.time(t) <= cur_time then
		t.day = t.day + 7
	end 
	return math.floor(os.time(t))
end 

-- 获取下一个月的时间
function dateutil.get_next_month_time(cur_time, hour, min, sec)

	local time = os.date("*t", cur_time)

	local year = time.month >= 12 and (time.year + 1) or time.year
	local month = (time.month < 12) and (time.month + 1) or 1

	local iNextMonthTime = math.floor(os.time({
		year = year,
		month = month,
		day = 1,
		hour = hour,
		min = min,
		sec = sec,
	}))

	return iNextMonthTime
end

function dateutil.isSameDay(timestampSrc, timestampDst)
	return dateutil.format_day(timestampSrc) == dateutil.format_day(timestampDst)
end

--(星期日到周六是1-7) iUpWeekDay: 1 - 7
function dateutil.isSameWeek(timestampSrc, timestampDst)
	local dateSrc = os.date("*t", timestampSrc)
	dateSrc.day = dateSrc.day - dateSrc.wday + 1
	
	local dateDst = os.date("*t", timestampDst)
	dateDst.day = dateDst.day - dateDst.wday + 1

	return dateutil.isSameDay(os.time(dateSrc), os.time(dateDst))
end

function dateutil.isSameMonth(timestampSrc, timestampDst)
	return os.date("*t", timestampSrc).month == os.date("*t", timestampDst).month
end

-- 返回(是否过了刷新时间, 距离刷新时间还有多久(秒)) iTime: 0 - 23
function dateutil.getFlashTimeData(nowtime, timestamp, iTime)

	assert(iTime >= 0 and iTime <= 23)
	nowtime, timestamp = math.floor(nowtime), math.floor(timestamp)
	local iFlashTime = dateutil.today_zero_seconds(dateutil.format_time_table(timestamp)) + iTime * 3600
	local iNowFlashTime = dateutil.today_zero_seconds(dateutil.format_time_table(nowtime)) + iTime * 3600

	if timestamp < iFlashTime then
		if nowtime < iFlashTime then
			return false, iFlashTime - nowtime
		else
			return true, (nowtime < iNowFlashTime) and (iNowFlashTime - nowtime) or (iNowFlashTime + 24 * 3600 - nowtime)
		end
	else
		local iNextFlashTime = iFlashTime + 24 * 3600
		if nowtime < iNextFlashTime then
			return false, iNextFlashTime - nowtime
		else
			return true, (nowtime < iNowFlashTime) and (iNowFlashTime - nowtime) or (iNowFlashTime + 24 * 3600 - nowtime)
		end
	end
end

-- 返回(是否过了刷新时间, 距离刷新时间还有多久(秒))(星期日到周六是1-7) iUpWeekDay: 1 - 7
function dateutil.getFlashWeekTimeData(nowtime, timestamp, iUpWeekDay)

	assert(iUpWeekDay >= 1 and iUpWeekDay <= 7)
	local iLastWeekDay = dateutil.get_last_weekday_time(timestamp, iUpWeekDay, 0, 0, 0)
	local iNextWeekDay = dateutil.get_next_weekday_time(timestamp, iUpWeekDay, 0, 0, 0)

	local iNowWeekDay = dateutil.today_zero_seconds(dateutil.format_time_table(nowtime))

	if iNowWeekDay < iLastWeekDay then
		return false, iLastWeekDay - nowtime
	elseif iNowWeekDay == iLastWeekDay then
		return false, iLastWeekDay + 24 * 3600 * 7 - nowtime
	elseif iNowWeekDay > iLastWeekDay and iNowWeekDay < iNextWeekDay then
		return false, iNextWeekDay - nowtime
	elseif iNowWeekDay == iNextWeekDay then
		return true, iNextWeekDay + 24 * 3600 * 7 - nowtime
	else
		local iFlashTime = dateutil.get_next_weekday_time(iNowWeekDay, iUpWeekDay, 0, 0, 0)
		return true, ((iNowWeekDay < (iNextWeekDay + 24 * 3600 * 7)) and (iNextWeekDay + 24 * 3600 * 7) or iFlashTime) - nowtime
	end
end

return util.ReadOnlyTable(dateutil)
