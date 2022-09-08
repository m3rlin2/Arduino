function split(str, sSeparator, nMax, bRegexp)
	assert(sSeparator ~= '')
	assert(nMax == nil or nMax >= 1)

	local aRecord = {}
	str = str or ""
	
	if str:len() > 0 then
		local bPlain = not bRegexp
		nMax = nMax or -1

		local nField, nStart = 1, 1
		local nFirst,nLast = str:find(sSeparator, nStart, bPlain)
		while nFirst and nMax ~= 0 do
			aRecord[nField] = str:sub(nStart, nFirst-1)
			nField = nField+1
			nStart = nLast+1
			nFirst,nLast = str:find(sSeparator, nStart, bPlain)
			nMax = nMax-1
		end
		aRecord[nField] = str:sub(nStart)
	end

	return aRecord
end

function sprintf(s,...)
	return s:format(...)
end

-- Module Interface

local Util = {}
Util.split = split
Util.sprintf = sprintf
return Util
