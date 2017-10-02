#pragma once
#include <list>

//------------------------------------------------------------------------------
template <typename... Args>
class Delegate {
public:
	Delegate()
	{
	}

	~Delegate()
	{
		clear();
	}

	void operator()(Args... args)
	{
		for (auto i = m_callables.begin(); i != m_callables.end();) {
			auto callable = *i;
			if (callable) {
				(*callable)(std::forward<Args>(args)...);
				++i;
			}
			else {
				i = m_callables.erase(i);
			}
		}
	}

	void add(void(*function)(Args...))
	{
		m_callables.emplace_back(new FunctionCallable<decltype(function)>(function));
	}
	
	template <typename Object>
	void add(Object* object, void (Object::*memberFunction)(Args...))
	{
		m_callables.emplace_back(new MemberFunctionCallable<Object, decltype(memberFunction)>(object, memberFunction));
	}

	void remove(void(*function)(Args...))
	{
		FunctionCallable<decltype(function)> callable(function);
		remove(callable);
	}

	template <typename Object>
	void remove(Object* object, void (Object::*memberFunction)(Args...))
	{
		MemberFunctionCallable<Object, decltype(memberFunction)> callable(object, memberFunction);
		remove(callable);
	}

	void clear()
	{
		for (auto callable : m_callables) {
			if (callable) {
				delete callable;
			}
		}
		m_callables.clear();
	}

	std::size_t getCount() const
	{
		return m_callables.size();
	}

	bool exists(void(*function)(Args...)) const
	{
		FunctionCallable<decltype(function)> callable(function);
		return exists(callable);
	}

	template <typename Object>
	bool exists(Object* object, void (Object::*memberFunction)(Args...)) const
	{
		MemberFunctionCallable<Object, decltype(memberFunction)> callable(object, memberFunction);
		return exists(callable);
	}
private:
	class Callable {
	public:
		virtual ~Callable() {}
		virtual void operator()(Args... args) const = 0;
		virtual bool operator==(const Callable& callable) const = 0;
	};

	template <typename Function>
	class FunctionCallable : public Callable {
	public:
		FunctionCallable(Function function):
			m_function(function)
		{
		}

		virtual void operator()(Args... args) const
		{
			m_function(std::forward<Args>(args)...);
		}

		virtual bool operator==(const Callable& callable) const
		{
			auto functionCallable = dynamic_cast<const FunctionCallable*>(&callable);
			if (functionCallable) {
				return m_function == functionCallable->m_function;
			}
			return false;
		}
	private:
		Function m_function;
	};

	template <typename Object, typename MemberFunction>
	class MemberFunctionCallable : public Callable {
	public:
		MemberFunctionCallable(Object* object, MemberFunction memberFunction) :
			m_object(object),
			m_memberFunction(memberFunction)
		{
		}

		virtual void operator()(Args... args) const
		{
			(m_object->*m_memberFunction)(std::forward<Args>(args)...);
		}

		virtual bool operator==(const Callable& callable) const
		{
			auto memberFunctionCallable = dynamic_cast<const MemberFunctionCallable*>(&callable);
			if (memberFunctionCallable) {
				return m_object == memberFunctionCallable->m_object && m_memberFunction == memberFunctionCallable->m_memberFunction;
			}
			return false;
		}
	private:
		Object* m_object;
		MemberFunction m_memberFunction;
	};

	typedef std::list<Callable*> Callables;
private:
	void remove(Callable& otherCallable)
	{
		for (auto i = m_callables.begin(); i != m_callables.end(); ++i) {
			auto& callable = *i;
			if (callable && *callable == otherCallable) {
				delete callable;
				callable = nullptr;
				break;
			}
		}
	}

	bool exists(Callable& otherCallable)
	{
		for (auto i = m_callables.begin(); i != m_callables.end(); ++i) {
			auto& callable = *i;
			if (callable && *callable == otherCallable) {
				return true;
			}
		}
		return false;
	}
private:
	Callables m_callables;
};

