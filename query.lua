function main(args)
	payload = {
		collection = "people",
		query = {
			_id = {["$oid"]="5ae8e0a004ed990001795403"}
		}		
	}

	return call("mongo", "query", payload)
end
