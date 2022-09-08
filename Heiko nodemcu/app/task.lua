local Topics = require "topics"
local Measurement = require "measurement"

local pre = "task: "
local msg_send 		= pre .. "send callback set"
local msg_init_1	= pre .. "initialization"
local msg_init_2	= pre .. "already running"

local firstRun = true
local timer = tmr.create()
local interval = 60*1000

function send(msg)
	print(msg)
end

function exec()
	if firstRun then
		firstRun = false
		send(msg_init_1)
		Measurement.meas()
		timer:register(interval, tmr.ALARM_AUTO, 
			function()
				Measurement.meas()
			end
		)
		timer:start()
	else
		send(msg_init_2)
	end
end

function setSendFunction(sendFunction)
	send = sendFunction
	send(msg_send)
	Measurement.setSendFunction(sendFunction)
end

-- Module interface

local Task = {}
Task.setSendFunction = setSendFunction
Task.exec = exec
return Task
