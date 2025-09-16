local buf = vim.api.nvim_get_current_buf()
local ns_metadata = vim.api.nvim_create_namespace("metadata")
local ns_hrule = vim.api.nvim_create_namespace("hrule")

local old_lines = vim.api.nvim_buf_get_lines(buf, 0, 10, false)

local function highlight_metadata()
	vim.api.nvim_buf_clear_namespace(buf, ns_metadata, 0, -1)

	local lines = vim.api.nvim_buf_get_lines(buf, 0, 10, false)
	if lines == old_lines then
		return
	end

	local valid_keywords = {
		source = true,
		topic = true,
		tags = true,
		url = true,
		desc = true,
		author = true,
		difficulty = true,
	}

	for lnum, line in ipairs(lines) do
		if line:match("^%%?%s*$") then
			goto continue
		end

		local commentstring, keyword = line:match("^(%%%s*)([a-zA-Z]+):")
		if not keyword then
			if vim.api.nvim_win_get_cursor(0)[1] == lnum then -- editing the current line
				goto continue
			end
			break
		end

		-- Highlight full line
		vim.api.nvim_buf_set_extmark(buf, ns_metadata, lnum - 1, #commentstring + #keyword + 1, {
			end_col = #line,
			hl_group = "Text",
			spell = false,
			priority = 101,
		})

		-- Highlight keyword
		local group = valid_keywords[keyword] and "Identifier" or "Error"
		vim.api.nvim_buf_set_extmark(buf, ns_metadata, lnum - 1, #commentstring, {
			end_col = #commentstring + #keyword,
			hl_group = group,
			spell = false,
			priority = 101,
		})
		valid_keywords[keyword] = false

		-- Highlight colon
		local colon_col = line:find(":")
		if colon_col then
			vim.api.nvim_buf_set_extmark(buf, ns_metadata, lnum - 1, colon_col - 1, {
				end_col = colon_col,
				hl_group = "Special",
				priority = 101,
			})
		end

		-- Highlight brackets and commas
		for i = 1, #line do
			local char = line:sub(i, i)
			if char == "[" or char == "]" or char == "," then
				vim.api.nvim_buf_set_extmark(buf, ns_metadata, lnum - 1, i - 1, {
					end_col = i,
					hl_group = "Delimiter",
					priority = 101,
				})
			end
		end
		::continue::
	end
end

local function highlight_hrule()
	vim.api.nvim_buf_clear_namespace(buf, ns_hrule, 0, -1)

	local lines = vim.api.nvim_buf_get_lines(buf, 0, -1, false)

	for lnum, line in ipairs(lines) do
		if line:match("^\\hrulebar%s*$") then
			vim.api.nvim_buf_set_extmark(buf, ns_hrule, lnum - 1, 0, {
				virt_text = { { string.rep("─", 80), "Indent" } },
				virt_text_pos = "overlay",
				hl_mode = "combine",
			})
		end
	end
end

if vim.env.OLY and not vim.b[buf].oly_highlight then
	vim.b[buf].oly_highlight = true

	highlight_metadata()
	highlight_hrule()

	vim.api.nvim_create_autocmd({ "TextChanged", "TextChangedI" }, {
		callback = function()
			highlight_metadata()
			highlight_hrule()
		end,
		buffer = buf,
	})
end
