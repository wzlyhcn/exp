#pragma once
#include <set>
#include <map>
#include <algorithm>
#include <cstring>

//------------------------------------------------------------------------------
class Delegatable;
class AbstractClosure;

//------------------------------------------------------------------------------
class AbstractDelegate {
	friend class Delegatable;
public:
	virtual ~AbstractDelegate() {}
protected:
	virtual void unbind(AbstractClosure* closure) = 0;
};

//------------------------------------------------------------------------------
class AbstractClosure {
public:
	virtual ~AbstractClosure() {}
};

//------------------------------------------------------------------------------
class Delegatable {
public:
	Delegatable()
	{
	}

	virtual ~Delegatable()
	{
		unbindAll();
	}

	void bind(AbstractDelegate* delegate, AbstractClosure* closure)
	{
		m_delegates[delegate].insert(closure);
	}

	void unbind(AbstractDelegate* delegate, AbstractClosure* closure)
	{
		auto i = m_delegates.find(delegate);
		if (i != m_delegates.end()) {
			Closures& closures = i->second;
			closures.erase(closure);
			if (closures.empty()) {
				m_delegates.erase(i);
			}
		}
	}

	void unbindAll()
	{
		for (auto i = m_delegates.begin(); i != m_delegates.end(); ++i) {
			AbstractDelegate* delegate = i->first;
			Closures& closures = i->second;
			for (auto j = closures.begin(); j != closures.end(); ++j) {
				AbstractClosure* closure = *j;
				delegate->unbind(closure);
			}
		}
		m_delegates.clear();
	}
private:
	typedef std::set<AbstractClosure*> Closures;
	typedef std::map<AbstractDelegate*, Closures> Delegates;
private:
	Delegates m_delegates;
};

//------------------------------------------------------------------------------
template <typename... Args>
class Delegate : public AbstractDelegate {
public:
	Delegate():
		m_currentChanged(false)
	{
		m_current = m_closures.end();
	}

	~Delegate()
	{
		clear();
	}

	void operator()(Args... args)
	{
		for (m_current = m_closures.begin(); m_current != m_closures.end();) {
			Closure* closure = *m_current;
			m_currentChanged = false;
			(*closure)(std::forward<Args>(args)...);
			if (!m_currentChanged) {
				++m_current;
			}
		}
	}

	template <typename Object>
	void bind(Object* object, void (Object::*memberFunction)(Args...))
	{
		MemberFunctionClosure<Object, decltype(memberFunction)> closure(object, memberFunction);
		auto i = m_closures.find(&closure);
		if (i == m_closures.end()) {
			Closure* _closure = closure.clone();
			m_closures.emplace(_closure);
			_closure->getDelegatable()->bind(this, _closure);
		}
	}

	template <typename Object>
	void unbind(Object* object, void (Object::*memberFunction)(Args...))
	{
		MemberFunctionClosure<Object, decltype(memberFunction)> closure(object, memberFunction);
		auto i = m_closures.find(&closure);
		if (i != m_closures.end()) {
			Closure* _closure = *i;
			_closure->getDelegatable()->unbind(this, _closure);
			delete _closure;
			if (m_currentChanged || i == m_current) {
				m_current = m_closures.erase(i);
				m_currentChanged = true;
			}
			else {
				m_closures.erase(i);
			}
		}
	}

	void clear()
	{
		for (auto i = m_closures.begin(); i != m_closures.end(); ++i) {
			Closure* _closure = *i;
			_closure->getDelegatable()->unbind(this, _closure);
			delete _closure;
		}
		m_closures.clear();
	}

	std::size_t getCount() const
	{
		return m_closures.size();
	}

	bool isEmpty() const
	{
		return m_closures.empty();
	}

	template <typename Object>
	bool exists(Object* object, void (Object::*memberFunction)(Args...)) const
	{
		MemberFunctionClosure<Object, decltype(memberFunction)> closure(object, memberFunction);
		return m_closures.find(&closure) != m_closures.end();
	}
protected:
	virtual void unbind(AbstractClosure* closure)
	{
		Closure* _closure = dynamic_cast<Closure*>(closure);
		auto i = m_closures.find(_closure);
		if (m_currentChanged || i == m_current) {
			m_current = m_closures.erase(i);
			m_currentChanged = true;
		}
		else {
			m_closures.erase(i);
		}
		delete _closure;
	}
private:
	class Closure : public AbstractClosure {
	public:
		virtual ~Closure() {}
		virtual Delegatable* getDelegatable() const = 0;
		virtual const void* getInternal() const = 0;
		virtual std::size_t getInternalSize() const = 0;
		virtual Closure* clone() const = 0;
		virtual void operator()(Args... args) const = 0;
	};

	struct ClosureLess {
		bool operator()(Closure* closure1, Closure* closure2) const
		{
			std::size_t size1 = closure1->getInternalSize();
			std::size_t size2 = closure2->getInternalSize();
			std::size_t size = std::min(size1, size2);
			int result = std::memcmp(closure1->getInternal(), closure2->getInternal(), size);
			if (result < 0) {
				return true;
			}
			else if (result > 0) {
				return false;
			}
			return size1 < size2;
		}
	};

	template <typename Object, typename MemberFunction>
	class MemberFunctionClosure : public Closure {
	public:
		MemberFunctionClosure(Object* object, MemberFunction memberFunction)
		{
			m_internal.object = object;
			m_internal.memberFunction = memberFunction;
		}

		virtual Delegatable* getDelegatable() const
		{
			return m_internal.object;
		}

		virtual const void* getInternal() const
		{
			return &m_internal;
		}

		virtual std::size_t getInternalSize() const
		{
			return sizeof(m_internal);
		}

		virtual Closure* clone() const
		{
			return new MemberFunctionClosure(m_internal.object, m_internal.memberFunction);
		}

		virtual void operator()(Args... args) const
		{
			(m_internal.object->*m_internal.memberFunction)(std::forward<Args>(args)...);
		}
	private:
		struct {
			Object* object;
			MemberFunction memberFunction;
		} m_internal;
	};

	typedef std::set<Closure*, ClosureLess> Closures;
private:
	Closures m_closures;
	typename Closures::iterator m_current;
	bool m_currentChanged;
};

