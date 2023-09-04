#pragma once
#include "Graphics/Bindable/Bindable.h"
#include <memory>
#include <unordered_map>

class BindableCodex {
public:
	template<class T, typename... Params>
	static std::shared_ptr<T> Resolve(Graphics& gfx, Params&&... p) dbgexcept {
		static_assert(std::is_base_of_v<Bindable, T>, "Can only resolve classes derived from Bindable");
		return Get().Resolve_<T>(gfx, std::forward<Params>(p)...);
	}
private:
	template<class T, typename... Params>
	std::shared_ptr<T> Resolve_(Graphics& gfx, Params&&... p) dbgexcept {
		const auto key = T::GenerateUID(std::forward<Params>(p)...);
		const auto i = binds.find(key);
		if (i == binds.end()) {
			auto bind = std::make_shared<T>(gfx, std::forward<Params>(p)...);
			binds[key] = bind;
			return bind;
		} else {
			return std::static_pointer_cast<T>(i->second);
		}
	}
	static BindableCodex& Get() {
		static BindableCodex codex;
		return codex;
	}
private:
	std::unordered_map<std::string, std::shared_ptr<Bindable>> binds;
};