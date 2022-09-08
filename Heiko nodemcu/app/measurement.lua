local Topics = require "topics"
local Util = require "util"

local pre = "meas: "
local msg_send		= pre .. "send callback set"
local msg_start		= pre .. "start"
local msg_err_crc	= pre .. "error checksum"
local msg_err_tim	= pre .. "error timeout"
local msg_err_wro	= pre .. "error data" 
local msg_meas		= pre .. "temp: %4.2f; humi: %4.2f"

function send(msg)
	print(msg)
end

function measurement()
	send(msg_start)
	pin = 1
	status, temp, humi = dht.read(pin)
	if humi > 150.0 then
		send(msg_err_wro)
	elseif status == dht.OK then
		send(Util.sprintf(msg_meas, temp, humi))
		send(temp, Topics.DHT.Meas.Temp, true)
		send(humi, Topics.DHT.Meas.Humi, true)
	elseif status == dht.ERROR_CHECKSUM then
		send(msg_err_crc)
	elseif status == dht.ERROR_TIMEOUT then
		send(msg_err_tim)
	end
end

function setSendFunction(sendFunction) 
	send = sendFunction
	send(msg_send)
end

-- Module interface

local Measurement = {}
Measurement.setSendFunction = setSendFunction
Measurement.meas = measurement
return Measurement
