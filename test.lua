#!/usr/bin/env lua

-- colors
local ERROR = "\27[38;5;196m"
local SUCCESS = "\27[38;5;46m"
local BOLD = "\27[1m"
local RESET = "\27[0m"

-- helpers
---@class OlyOpts
---@field silent? boolean

---Runs an oly command
---@param cmd string[]
---@param opts OlyOpts?
---@return file*?
local function oly(cmd, opts)
	local cmd = "build/Debug/oly " .. table.concat(cmd, " ")
	opts = opts or {}
	if opts.silent then
		cmd = cmd .. " >/dev/null 2>&1"
	end
	local out = io.popen(cmd, "r")
	if out then
		return out
	else
		io.write(ERROR .. "popen failed !" .. RESET)
		return nil
	end
end

-- tests
local tests = {
	{
		name = "problem retrieval",
		test = function()
			local failed = {}
			for _, lang in ipairs({ "latex", "typst" }) do
				local handle = oly({ "list", "--filter-lang", "--lang", lang })
				if not handle then return end
				for pb in handle:lines() do
					local show = oly({ "show", '"' .. pb .. '"', "--lang", lang }, { silent = true })
					if not show then return end
					local ok, reason, code = show:close()
					if not ok and code ~= 134 then -- ignore aborts
						table.insert(failed, pb)
					end
				end
				handle:close()
			end

			local first = true
			for _, pb in ipairs(failed) do
				if first then
					first = false
				else
					io.write(", ")
				end
				io.write(ERROR .. pb .. RESET)
			end
			if #failed > 0 then
				io.write(" failed !\n")
				return 1
			end
		end,
	},
	{
		name = "problem naming",
		test = function()
			local handle = oly({ "list", "--test" })
			if not handle then return end
			local out = handle:read("*a")
			if out ~= "" then
				io.write("Problem names not matching their source:\n")
				io.write(ERROR .. out .. RESET)
				return 1
			end
			handle:close()
		end,
	},
}

-- run tests
for i, test in ipairs(tests) do
	print(BOLD .. "Testing " .. test.name .. "..." .. RESET)
	if test.test() == nil then
		print(SUCCESS .. "All good !" .. RESET)
	end

	if i ~= #tests then
		print()
	end
end
