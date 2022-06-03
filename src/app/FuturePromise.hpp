
#ifndef PG_BSK_PROJECT_FUTURE_PROMISE_HPP
#define PG_BSK_PROJECT_FUTURE_PROMISE_HPP

#include <future>
#include <memory>
#include <exception>
#include <functional>

class FuturePromiseException: public std::runtime_error{using std::runtime_error::runtime_error;};

template<typename Arg>
class Future {
public:
	
	Future(std::shared_future<Arg> argFuture) : argFuture(argFuture) {
	}
	
	bool Valid() {
		return argFuture.valid();
	}
	
	void Wait() {
		argFuture.wait();
	}
	
	Arg Get() {
		return argFuture.get();
	}
	
	template<typename Ret>
	Future<Ret> Then(std::function<Ret(Arg)> callback) {
		if(argFuture.valid()) {
			return std::async( [callback](std::shared_future<Arg> arg)->Ret {
					return callback(arg.get());
					}, argFuture).share();
		} else {
			throw FuturePromiseException("Future<>::Then<>()::else is not implemented.");
			/*
			std::promise<Ret> promise;
			promise.set_exception(new FuturePromiseException("invalid std::shared_future state when using Future<...>::Then()"));
			return Future(promise.get_future().shared());
			*/
		}
	}
	
private:
	
	std::shared_future<Arg> argFuture;
	
};

template<>
class Future<void> {
public:
	
	Future(std::shared_future<void> argFuture) : argFuture(argFuture) {
	}
	
	bool Valid() {
		return argFuture.valid();
	}
	
	void Wait() {
		argFuture.wait();
	}
	
	void Get() {
		argFuture.wait();
	}
	
	template<typename Ret>
	Future<Ret> Then(std::function<Ret(void)> callback) {
		if(argFuture.valid()) {
			return std::async( [callback](std::shared_future<void> arg)->Ret {
					arg.wait();
					return callback();
					}, argFuture).share();
		} else {
			throw FuturePromiseException("Future<>::Then<>()::else is not implemented.");
			/*
			std::promise<Ret> promise;
			promise.set_exception(new FuturePromiseException("invalid std::shared_future state when using Future<...>::Then()"));
			return Future(promise.get_future().shared());
			*/
		}
	}
	
private:
	
	std::shared_future<void> argFuture;
	
};

template<typename Arg>
class Promise {
public:
	
	Promise() {}
	
	Future<Arg> GetFuture() {
		return Future<Arg>(promise.get_future().share());
	}
	
	Promise<Arg>& SetValue(Arg value) {
		promise.set_value(value);
		return *this;
	}
	
	Promise<Arg>& SetException(std::exception_ptr exception) {
		promise.set_exception(exception);
		return *this;
	}
	
private:
	
	std::promise<Arg> promise;
};

#endif

