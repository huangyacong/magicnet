local CoreMySql = require "CoreMySql"
local util = require "util"

local CCoreMysql = {}

-- 执行
local function executesql(iSqlId, cSql)
	-- 执行
	local bRet = CoreMySql.ExecuteSql(iSqlId, cSql)

	if not bRet then
		print(debug.traceback(), "\n", string.format("[SQL] ExecuteSql Failed. errormsg=%s code=%s.", CCoreMysql.errorstr(iSqlId), CCoreMysql.errorno(iSqlId)))
		print(string.format("[SQL] ExecuteSql Failed. sql=%s", cSql))
	end

	if bRet then
		return bRet
	end

	if not CCoreMysql.isconnect(iSqlId) then
		CCoreMysql.disconnect(iSqlId)
		print("[SQL] dbsvr is disconnect, now reconnect it.")

		if not CCoreMysql.tryconect(iSqlId) then
			print(debug.traceback(), "\n", string.format("[SQL] dbsvr reconnect error. errormsg=%s code=%s", CCoreMysql.errorstr(iSqlId), CCoreMysql.errorno(iSqlId)))
			print(string.format("[SQL] dbsvr connect error. sql=%s", cSql))
			return false
		end

		print("[SQL] dbsvr is disconnect, reconnect ok.")

		if not CoreMySql.ExecuteSql(iSqlId, cSql) then
			print(debug.traceback(), "\n", string.format("[SQL] ExecuteSql Failed. errormsg=%s code=%s", CCoreMysql.errorstr(iSqlId), CCoreMysql.errorno(iSqlId)))
			print(string.format("[SQL] ExecuteSql Failed. sql=%s", cSql))
			return false
		end

		return true
	end

	return bRet
end

-- 加载
function CCoreMysql.loadmysqldll()
	return CoreMySql.LibraryInit()
end

-- 进程退出
function CCoreMysql.downmysqldll()
	return CoreMySql.LibraryEnd()
end

-- 初始化
function CCoreMysql.create(iSqlId, cHost, iPort, cDBName, cUser, cPwd, cCharacter)
	return CoreMySql.CreateSqlInstance(iSqlId, cHost, iPort, cDBName, cUser, cPwd, cCharacter)
end

-- 链接
function CCoreMysql.tryconect(iSqlId)
	return CoreMySql.TryConnect(iSqlId)
end

-- 是否链接中
function CCoreMysql.isconnect(iSqlId)
	return CoreMySql.IsConnect(iSqlId)
end

-- 断线
function CCoreMysql.disconnect(iSqlId)
	return CoreMySql.DisConnect(iSqlId)
end

-- 修改
function CCoreMysql.modifysql(iSqlId, cSql)
	if not executesql(iSqlId, cSql) then return false, 0, 0 end
	return true, CoreMySql.GetLastInsertId(iSqlId), CoreMySql.GetAffectedRows(iSqlId)
end

-- 获取
function CCoreMysql.selectsql(iSqlId, cSql)
	if not executesql(iSqlId, cSql) then return nil end
	return CoreMySql.StoreResult(iSqlId)
end

-- 过滤
function CCoreMysql.escapesql(iSqlId, args)
	return CoreMySql.Escape(iSqlId, args)
end

-- 设置自动提交
function CCoreMysql.setautocommit(iSqlId, flag)
	return CoreMySql.SetAutoCommit(iSqlId, flag)
end 

-- 写入事务
function CCoreMysql.commit(iSqlId)
	return CoreMySql.Commit(iSqlId)
end

-- 回滚事务
function CCoreMysql.rollback(iSqlId)
	return CoreMySql.Rollback(iSqlId)
end

-- 获取错误代码
function CCoreMysql.errorno(iSqlId)
	return CoreMySql.GetErrorCode(iSqlId)
end

-- 获取错误字符串
function CCoreMysql.errorstr(iSqlId)
	return CoreMySql.GetErrorStr(iSqlId)
end

return util.ReadOnlyTable(CCoreMysql)
