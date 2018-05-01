function main(args)
	local payload = {
		collection="people",
		data={
			name="Homer",
			age=40
		}
	}

	return call("mongo", "insert", payload)
end
