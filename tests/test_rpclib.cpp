
#include "../rpclib/include/rpc/client.h"
#include "../rpclib/include/rpc/server.h"

#include <iostream>

void foo() {
	std::cout << "rpclib ... OK" << std::endl;
}

int main(int argc, char *argv[]) {
	rpc::server srv(8080);
	srv.bind("foo", &foo);
	srv.bind("add", [](int a, int b) {
		return a + b;
	});
	srv.bind("exit", [](int code){ exit(code); });

	srv.async_run();
	
	
	rpc::client client("127.0.0.1", 8080);

	// Calling a function with paramters and converting the result to int
	client.send("foo");
	auto result = client.call("add", 2, 3).as<int>();
	std::cout << "The result is: " << result << std::endl;
	client.send("exit", 1);
	return 0;

	return 0;
}

