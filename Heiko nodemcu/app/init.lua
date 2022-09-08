dofile("credentials.lua")

local Application = require "application"

function startup()
	print("Startup...")
	Application.run()
end

function wifi_connect_event(T) 
	print("Connection to AP("..T.SSID..") established!")
	print("Waiting for IP address...")
	if disconnect_ct ~= nil then disconnect_ct = nil end	
end

function wifi_got_ip_event(T)		 
	print("Wifi connection is ready! IP address is: "..T.IP)
	print("NTP sync")
	sntp.sync("time.google.com", 
		function(sec, usec, server, info)
			print("NTP success", sec, usec, server)
		end, 
		function() 
			print("NTP failure") 
		end, 
		1
	)
	startup()
end

function wifi_disconnect_event(T)
	if T.reason == wifi.eventmon.reason.ASSOC_LEAVE then 
		--the station has disassociated from a previously connected AP
		return 
	end
	-- total_tries: how many times the station will attempt to connect to the AP. Should consider AP reboot duration.
	local total_tries = 100
	print("\nWiFi connection to AP("..T.SSID..") has failed!")

	--There are many possible disconnect reasons, the following iterates through 
	--the list and returns the string corresponding to the disconnect reason.
	for key,val in pairs(wifi.eventmon.reason) do
		if val == T.reason then
			print("Disconnect reason: "..val.."("..key..")")
			break
		end
	end

	if disconnect_ct == nil then 
		disconnect_ct = 1 
	else
		disconnect_ct = disconnect_ct + 1 
	end
	if disconnect_ct < total_tries then 
		print("Retrying connection...(attempt "..(disconnect_ct+1).." of "..total_tries..")")
	else
		wifi.sta.disconnect()
		print("Aborting connection to AP!")
		disconnect_ct = nil 
		node.restart()
	end
end

wifi.eventmon.register(wifi.eventmon.STA_CONNECTED, wifi_connect_event)
wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, wifi_got_ip_event)
wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, wifi_disconnect_event)

print("Connecting to WiFi access point...")
wifi.setmode(wifi.STATION)
wifi.sta.config({ssid=WLAN_SSID, pwd=WLAN_PASSWORD}) -- autoconnect
