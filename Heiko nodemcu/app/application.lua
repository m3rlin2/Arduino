dofile("credentials.lua")

local Topics = require "topics"
local Util = require "util"
local OTA = require "ota"
local Task = require "task"
 
local pre = "app: "
local msg_app_init_1	= pre .. "initialization"
local msg_app_init_2	= pre .. "already running"
pre = "mqtt: "
local msg_try_conn		= pre .. "trying to connect %d/%d"
local msg_conn			= pre .. "connected"
local msg_disconn		= pre .. "disconnected"
local msg_error			= pre .. "error"
local msg_lwt			= pre .. "lwt"

local firstRun = true
local connected = false
local conCnt = 0
local maxConCnt = 20
local mqttClient = nil

function timed()
	return rtctime.get() .. ": "
end

function send(msg, topic, notStdOut)
	topic = topic or Topics.DHT.Info
	if not notStdOut then
		if topic == Topics.DHT.Info then
			print(timed() .. msg)
		else
			print(timed() .. msg .. "\t(topic: " .. topic .. "; connected: " .. tostring(connected) ..")")
		end
	end
	if mqttClient and connected then
		mqttClient:publish(topic, msg, 0, 0)
	end
end

function initMqttClient()
	client = mqtt.Client("DHT22", 60, MQTT_USER, MQTT_PASSWORD)
	client:on("offline", 
		function(client) 
			connected = false
			send(msg_disconn)
			connect()
		end
	)
	client:lwt(Topics.DHT.Info, msg_lwt, 0, 0)
	client:on("message", 
		function(client, topic, data) 
			local printIt = true
			if topic == Topics.DHT.Cmd.Restart and data == "0" then
				node.restart()
			end
			if topic == Topics.DHT.Cmd.Update then
				printIt = false
				OTA.update(data)
			end
			if printIt then
				send(topic .. ": " .. data)
			end
		end
	)
	return client
end

function connectionSuccessful(client)
	connected = true
	conCnt = 0
	send(msg_conn)
	
	send("1", Topics.DHT.Cmd.Restart)
	mqttClient:subscribe(Topics.DHT.Cmd.Restart, 0)
	mqttClient:subscribe(Topics.DHT.Cmd.Update, 0)
	
	Task.exec()
end

function connectionFailed(client, reason)
	send(msg_error .. ": " .. reason)
	connect()
end

function connect()
	mqttClient = initMqttClient()
	
	conCnt = conCnt + 1
	if conCnt > maxConCnt then
		node.restart()
	end
	send(Util.sprintf(msg_try_conn, conCnt, maxConCnt))
	
	mqttClient:connect(MQTT_ADDR, MQTT_PORT, 0, connectionSuccessful, connectionFailed)
end

function run() 
	if firstRun then
		firstRun = false
		send(msg_app_init_1)
		connect()
		
		OTA.setSendFunction(send)
		Task.setSendFunction(send)
	else
		send(msg_app_init_2)
		mqttClient:close() -- force reconnection
	end
end

-- Module interface

local Application = {}
Application.run = run
return Application
