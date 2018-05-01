function main(args)
	payload = {
		collection = "people",
		query = {
			_id = {["$oid"]="5ae8d4c5b31fc8741f2de073"}
		}		
	}

	return call("mongo", "query", payload)
end
