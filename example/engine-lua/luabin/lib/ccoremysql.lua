local CoreMySql = require "CoreMySql"
local util = require "util"

local ccoremysql = {}

-- 执行
local function executesql(cSql)
	-- 执行
	local bRet = CoreMySql.ExecuteSql(cSql)

	if not bRet then
		print(debug.traceback(), "\n", string.format("[SQL] ExecuteSql Failed. errormsg=%s.", ccoremysql.errorstr()))
		print(string.format("[SQL] ExecuteSql Failed. sql=%s", cSql))
	end

	if bRet then
		return bRet
	end

	if not ccoremysql.isconnect() then
		ccoremysql.disconnect()
		print("[SQL] dbsvr is disconnect, now reconnect it.")

		if not ccoremysql.tryconect() then
			print(debug.traceback(), "\n", string.format("[SQL] dbsvr reconnect error. errormsg=%s", ccoremysql.errorstr()))
			print(string.format("[SQL] dbsvr connect error. sql=%s", cSql))
			return false
		end

		print("[SQL] dbsvr is disconnect, reconnect ok.")

		if not CoreMySql.ExecuteSql(cSql) then
			print(debug.traceback(), "\n", string.format("[SQL] ExecuteSql Failed. errormsg=%s", ccoremysql.errorstr()))
			print(string.format("[SQL] ExecuteSql Failed. sql=%s", cSql))
			return false
		end

		return true
	end

	return bRet
end

-- 加载
function ccoremysql.loadmysqldll()
	return CoreMySql.LibraryInit()
end

-- 进程退出
function ccoremysql.downmysqldll()
	return CoreMySql.LibraryEnd()
end

-- 初始化
function ccoremysql.init(cHost, iPort, cDBName, cUser, cPwd, cCharacter)
	return CoreMySql.Init(cHost, iPort, cDBName, cUser, cPwd, cCharacter)
end

-- 链接
function ccoremysql.tryconect()
	return CoreMySql.TryConnect()
end

-- 是否链接中
function ccoremysql.isconnect()
	return CoreMySql.IsConnect()
end

-- 断线
function ccoremysql.disconnect()
	return CoreMySql.DisConnect()
end

-- 修改
function ccoremysql.modifysql(cSql)
	if not executesql(cSql) then return false, 0, 0 end
	return true, CoreMySql.GetLastInsertId(), CoreMySql.GetAffectedRows()
end

-- 获取
function ccoremysql.selectsql(cSql)
	if not executesql(cSql) then return nil end
	return CoreMySql.StoreResult()
end

-- 过滤
function ccoremysql.escapesql(args)
	return CoreMySql.Escape(args)
end

-- 写入事务
function ccoremysql.commit()
	return CoreMySql.Commit()
end

-- 回滚事务
function ccoremysql.rollback()
	return CoreMySql.Rollback()
end

-- 获取错误代码
function ccoremysql.errorno()
	return CoreMySql.GetErrorCode()
end

-- 获取错误字符串
function ccoremysql.errorstr()
	return CoreMySql.GetErrorStr()
end

return util.ReadOnlyTable(ccoremysql)
