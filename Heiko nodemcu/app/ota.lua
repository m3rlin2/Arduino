local Topics = require "topics"
local Util = require "util"

local pre = "ota: "
local msg_send				= pre .. "send callback set"
local msg_update_start		= pre .. "update started"
local msg_update_err		= pre .. "update error"
local msg_update_repl		= pre .. "save old "
local msg_update_write		= pre .. "write data to "
local msg_update_success	= pre .. "update finished"

local msg_collecting		= pre .. "collecting %d/%d from '%s'"
local msg_err_compare		= pre .. "error compare %d/%d from '%s'"
local msg_err_tmp			= pre .. "error write to tmp file '%s' failed"

local m_n = 0
local m_N = 0
local m_name = ""
local m_data = ""
local m_collecting = false

function send()
	print(msg)
end

function printFiles(after)
	str = (after and "after") or "before"
	str = str .. " update:"
	print("Filesystem " .. str)
	l = file.list();
	for k,v in pairs(l) do
		print("\tname: "..k..", \tsize: "..v)
	end
end

function update(name, nameTmp, keep)
	send(msg_update_start)
	if name:len() > 0 and nameTmp:len() > 0 then
		printFiles()
		-- If file exists, save it
		if keep and file.exists(name) then
			send(msg_update_repl .. name)
			old = name .. ".old"
			file.remove(old)
			file.rename(name, old)
		end
		-- Write new content
		send(msg_update_write .. name)
		if file.exists(nameTmp) then
			file.rename(nameTmp, name)
		end
		printFiles(true)
		send(msg_update_success)
	else
		send(msg_update_err 
			.. "; nameTmp:  '" .. nameTmp .. "'"
			.. "; name:  '" .. name .. "'")
	end
	reset()
end

function reset()
	file.remove(m_name .. ".tmp")
	m_n = 0
	m_N = 0
	m_name = ""
	m_collecting = false
end

function compare(name, n, N, nameTmp, data)
	m_n = m_n + 1
	ok = m_name == name and m_n == n and m_N == N
	if ok then
		send(Util.sprintf(msg_collecting, n, N, name))
		ok = saveToTmp(nameTmp, data)
	else
		send(Util.sprintf(msg_err_compare, n, N, name))
	end
	return ok
end

function saveToTmp(tmp, data)
	local fd = file.open(tmp, "a")
	if fd then
		fd:write(data)
		fd:close()
		return true
	else
		send(Util.sprintf(msg_err_tmp, tmp))
		reset()
		return false
	end
end
	
function collect(data)
	local splitted = Util.split(data, '#', 3)
	local splittedPart = Util.split(splitted[1], '/', 1)
	
	local n = tonumber(splittedPart[1] or "0")
	local N = tonumber(splittedPart[2] or "0")
	
	local name = splitted[2] or ""
	local nameTmp = name .. ".tmp"
	local keep = splitted[3] == "true"
	local data = splitted[4] or ""
	
	if n == 0 or N == 0 or name:len() == 0 or data:len() == 0 then
		reset()
		return
	end
	
	if not m_collecting then
		reset()
		if n == 1 and N == 1 then
			if saveToTmp(nameTmp, data) then
				update(name, nameTmp, keep)
			end
		elseif n == 1 and n < N then
			send(Util.sprintf(msg_collecting, n, N, name))
			if not saveToTmp(nameTmp, data) then
				return
			end
			m_collecting = true
			m_name = name
			m_n = n
			m_N = N
		else
			reset()
		end
	else
		if compare(name, n, N, nameTmp, data) then
			if n == N then
				update(name, nameTmp, keep)
			end
		else
			reset()
		end
	end
	
end
 
function setSendFunction(sendFunction) 
	send = sendFunction
	send(msg_send)
end

-- Module interface

local OTA = {}
OTA.setSendFunction = setSendFunction
OTA.update = collect
return OTA
