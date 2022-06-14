
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
// 	Future(Future&) = default;
	Future(Future&&) = default;
	
	inline bool Valid() {
		return argFuture.valid();
	}
	
	inline void Wait() {
		argFuture.wait();
	}
	
	inline Arg Get() {
		return argFuture.get();
	}
	
	Future<void> Then(std::function<void(Arg)> callback);
	template<typename Ret>
	Future<Ret> Then(std::function<Ret(Arg)> callback);
	
private:
	
	std::shared_future<Arg> argFuture;
	
};

template<>
class Future<void> {
public:
	
	Future(std::shared_future<void> argFuture) : argFuture(argFuture) {
	}
// 	Future(Future&) = default;
	Future(Future&&) = default;
	
	inline bool Valid() {
		return argFuture.valid();
	}
	
	inline void Wait() {
		argFuture.wait();
	}
	
	inline void Get() {
		argFuture.wait();
	}
	
	Future<void> Then(std::function<void(void)> callback);
	template<typename Ret>
	Future<Ret> Then(std::function<Ret(void)> callback);
	
private:
	
	std::shared_future<void> argFuture;
	
};

template<typename Arg>
class Promise {
public:
	
	Promise() {}
// 	Promise(Promise&) = default;
	Promise(Promise&&) = default;
	
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

template<>
class Promise<void> {
public:
	
	Promise() {}
// 	Promise(Promise&) = default;
	Promise(Promise&&) = default;
	
	inline Future<void> GetFuture() {
		return Future<void>(promise.get_future().share());
	}
	
	inline Promise<void>& SetValue() {
		promise.set_value();
		return *this;
	}
	
	inline Promise<void>& SetException(std::exception_ptr exception) {
		promise.set_exception(exception);
		return *this;
	}
	
private:
	
	std::promise<void> promise;
};

inline Future<void> Future<void>::Then(std::function<void(void)> callback) {
	if(argFuture.valid()) {
		/*std::shared_ptr<Promise<void>> p = std::make_shared<Promise<void>>();
		auto pr = argFuture;
		std::thread t([callback, p, pr]() {
			pr.wait();
			callback();
			p->SetValue();
		});
		t.detach();
		return p->GetFuture();*/
		
		return std::async(std::launch::async,
				[callback](std::shared_future<void> arg)->void {
				arg.wait();
				return callback();
				}, argFuture).share();
				
	} else {
		throw FuturePromiseException("Future<>::Then<>()::else is not implemented.");
// 		   std::promise<Ret> promise;
// 		   promise.set_exception(new FuturePromiseException("invalid std::shared_future state when using Future<...>::Then()"));
// 		   return Future(promise.get_future().shared());
	}
}

template<typename Ret>
inline Future<Ret> Future<void>::Then(std::function<Ret(void)> callback) {
	if(argFuture.valid()) {
		/*std::shared_ptr<Promise<Ret>> p = std::make_shared<Promise<Ret>>();
		auto pr = argFuture;
		std::thread t([callback, p, pr]() {
			pr.wait();
			p->SetValue(callback());
		});
		t.detach();
		return p->GetFuture();*/
		
		return std::async(std::launch::async,
				[callback](std::shared_future<void> arg)->Ret {
				arg.wait();
				return callback();
				}, argFuture).share();
				
	} else {
		throw FuturePromiseException("Future<>::Then<>()::else is not implemented.");
// 		   std::promise<Ret> promise;
// 		   promise.set_exception(new FuturePromiseException("invalid std::shared_future state when using Future<...>::Then()"));
// 		   return Future(promise.get_future().shared());
	}
}

template<typename Arg>
template<typename Ret>
inline Future<Ret> Future<Arg>::Then(std::function<Ret(Arg)> callback) {
	if(argFuture.valid()) {
		/*std::shared_ptr<Promise<Ret>> p = std::make_shared<Promise<Ret>>();
		auto pr = argFuture;
		std::thread t([callback, p, pr]() {
				pr.wait();
				p->SetValue(callback(pr.get()));
				});
		t.detach();
		return p->GetFuture();*/
		
		return std::async(std::launch::async,
				[callback](std::shared_future<Arg> arg)->Ret {
				return callback(arg.get());
				}, argFuture).share();
				
	} else {
		throw FuturePromiseException("Future<>::Then<>()::else is not implemented.");
// 		   std::promise<Ret> promise;
// 		   promise.set_exception(new FuturePromiseException("invalid std::shared_future state when using Future<...>::Then()"));
// 		   return Future(promise.get_future().shared());
	}
}

template<typename Arg>
inline Future<void> Future<Arg>::Then(std::function<void(Arg)> callback) {
	if(argFuture.valid()) {
		/*std::shared_ptr<Promise<void>> p = std::make_shared<Promise<void>>();
		auto pr = argFuture;
		std::thread t([callback, p, pr]() {
				pr.wait();
				callback(pr.get());
				p->SetValue();
				});
		t.detach();
		return p->GetFuture();*/
		
		return std::async(std::launch::async,
				[callback](std::shared_future<Arg> arg)->void {
				return callback(arg.get());
				}, argFuture).share();
		
	} else {
		throw FuturePromiseException("Future<>::Then<>()::else is not implemented.");
// 		   std::promise<Ret> promise;
// 		   promise.set_exception(new FuturePromiseException("invalid std::shared_future state when using Future<...>::Then()"));
// 		   return Future(promise.get_future().shared());
	}
}

#endif

