local Topics = {}

Topics.DHT = {}
Topics.DHT.Cmd = {}
Topics.DHT.Meas = {}
Topics.DHT.Update = {}

local pre = "/home/living/dht"
local cmd = pre .. "/cmd"

Topics.DHT.Info = pre .. "/info"

Topics.DHT.Cmd.Restart = cmd .. "/restart"
Topics.DHT.Cmd.Update = cmd .. "/update"

Topics.DHT.Meas.Temp = pre .. "/meas/temp"
Topics.DHT.Meas.Humi = pre .. "/meas/humi"

return Topics
