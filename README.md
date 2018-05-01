# Experimental Function-as-a-Service Server

An Openwhisk like server based on Lua and libuv. Very small memory footprint (binary is ~2.7M, memory usage is usually < 10M). Mongodb is used for persistence.

*All of this is purely experimental / for fun and not at all usable in production*

## How does it work

The server can run `actions` where every action is a snippet of Lua code. When a new action is added it will be compiled and stored as bytecode in Mongodb.
When an action is run, it's bytecode will be retrieved and the `main` function will be invoked with the submitted parameters. Code execution always happens in a Sandbox.
The Sandbox disables dangerous APIs and can enforce memory (in kilobytes) and runtime (in milliseconds) restrictions for every action.

Actions also have access to [plugins](https://github.com/pb82/lua-experiments/tree/master/libs) through the `call` API. Plugins are shared libraries that are loaded when the server starts (and they are configured).
Configuration is done in Lua which allows for a [very nice config syntax](https://github.com/pb82/lua-experiments/blob/master/config.lua).

## How to interact with the server

The server exposes a http API (port 8080) that is designed to work with it's [CLI](https://www.npmjs.com/package/@pb82/slcli). You can also use cURL but the CLI makes things easier.
You can add, delete and run actions. When running an action you can either wait for the result or run it asynchronously.

## Example

Quick walktrough:

### Install the CLI

With yarn:

```sh
$ yarn global add @pb82/slcli
```

or npm:

```sh
$ npm install -g @pb82/slcli
```

The code for the CLI can be found [here](https://github.com/pb82/lua-experiments/tree/master/cli).

### Run the Server

It's best to use the provided [docker compose file](https://github.com/pb82/lua-experiments/blob/master/docker-compose.yaml). Copy this file to some location and run `docker-compose up`.
This will start two containers, the Mongodb database and the Server itself.
That's it, you're ready to go.

### Basic cli commands

First you will need to connect to the server:

```sh
$ slcli connect http://localhost:8080
info: Operation successful
```

At this point you should see a `Ping request received` entry in the server logs. You can send another ping with:

```sh
$ slcli ping
```

To get a list of all available actions run:

```sh
$ slcli list
info: Available actions
[]
```

If this is your first start this should be empty. Now let's add a new action. Create a file named `hello.lua`:

```lua
function main(args)
	if args.name == nil then
		args.name = 'World'
	end
	
	return 'Hello ' .. args.name
end
```

and add it as an action using:

```sh
$ slcli create hello -f hello.lua 
```

Create another file named `payload.json`:

```json
{
	"name": "from Lua"
}
```

Finally, run the action:

```
$ slcli run hello -b -p payload.json
info: Operation successful
Hello from Lua
```

The payload is transformed to a Lua table and passed to the `main` function as `args`. The `-b` flag means 'block`. Use it to wait until the result is received.
If you run the command without the `-b` flag you will get an `invocation ID`:

```sh
$ slcli run hello -p payload.json
info: Operation successful
{ InvocationId: <your ID> }
```

This will return immediately and you can use the invocation ID to retrieve the result anytime you want:

```sh
$ slcli resolve <your ID>
Hello from Lua
```

Now try:

```sh
$ slcli list
[ { maxmem: 0, name: 'hello', size: 192, timeout: 0 } ]
```

You can see the available actions (only hello). `size` is the bytecode size of the action in bytes. There are two other interesting properties:

* `maxmem`: The maximum amount of memory (RAM) the action can use (in kilobytes)
* `timeout`: The maximum amount of time an action is allowed to take (in milliseconds)

Let's try this. Create another file named `evil.lua`:

```lua
function main(args)
	strings = {}
	while true do
		table.insert(strings, 'evil')
	end
end
```

This will allocate strings in an endless loop consuming lots of memory (Lua is garbage collected but can't collect the `strings` table here because the reference is still valid.

Create the action using the `-m` flag which allows us to set a memory limit:

```sh
$ slcli create evil -f evil.lua -m 100000
```

In this case we set 100 Megabytes as the limit. Now run the action and observe the server logs:

```
$ slcli run evil -b
error: Error: Request failed with status code 500
```
You should see the 500 error in the console after a second or two and you should see the following entry in the server logs: `Action 'evil' aborted after violating memory constraints`.
If you want to try a timeout too you can use the `-t` flag.

## Calling plugins

Running isolated Lua code would be pretty useles if it could not interact with external services. This is achieved through plugins (in contrast to Openwhisk which allows you to basically run a whole project with all dependencies and do whatever you want).
For this POC only two plugins exist: [skeleton](https://github.com/pb82/lua-experiments/tree/master/libs/skeleton) which is just there to showcase the plugin API.
And [mongo](https://github.com/pb82/lua-experiments/tree/master/libs/mongo) which allows you to talk to a real mongo database. Plugins are configured in [config.lua](https://github.com/pb82/lua-experiments/tree/master/libs/mongo) but if you use docker compose everything is pre-confgured to use the included Mongo instance.

Inside your action you can use `call` to talk to interact with plugins. `call` takes three arguments:

```
call(pluginName: <string>, action: <string>, payload <any>)
```

`pluginName` should be obvious. `action` can be used to differentiate between intents in the plugin (the mongo plugin accepts `insert` and `query`). Payload can be any value and will be passed to the plugin.

Let's write an action that inserts a person:

```lua
function getAge()
	return math.floor(math.random() * 100)
end

function main(args)
	if args.name == nil then
		args.name = 'noname'
	end

	local payload = {
		collection = 'mongoplugin',
		data = {
			name = args.name,
			age = getAge()
		}
	}

	return call('mongo', 'insert', payload)
end
```

and create it with `$ slcli create insert -f insert.lua`

Run this a few times:

```sh
$ slcli run insert -b
5ae8f10fbb57530001473b36
```

The `insert` intent will return the ObjectID of the inserted document. Now let's write an action to query the data:

```lua
function main(args)
	local payload = {
		collection = 'mongoplugin',
		query = {
			age = {
				['$gt'] = 50
			}
		}
	}

	return call('mongo', 'query', payload)
end
```

Lua tables can be used in a similar way to JSON objects which suits Mongodb very well. Create & run it for some results:

```sh
$ slcli create query -f query.lua
$ slcli run query -b
info: Operation successful
{ '1': 
   { _id: { '$oid': '5ae8f10fbb57530001473b36' },
     age: 78,
     name: 'noname' },
  '2': 
   { _id: { '$oid': '5ae8f1a2bb57530001473b37' },
     age: 79,
     name: 'noname' },
  '3': 
   { _id: { '$oid': '5ae8f1a3bb57530001473b38' },
     age: 91,
     name: 'noname' }
```

The CLI has one other feature that has not been covered: `delete` to delete actions (`$ slcli delete <action name>`).

## How fast is it?

The code is all quick and dirty and completely unoptimized. I did some benchmarking with `wrk` and on my 7 year old Desktop PC i get ~3000 requests per second.
This could be improved easily by caching actions (the bytecode is loaded from mongo every time) and fixing some of the locking issues.