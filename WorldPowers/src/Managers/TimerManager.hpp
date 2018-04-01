#pragma once
#include <unordered_map>
#include <functional>
#include <memory>

#include "Common.hpp"
#include "Time/Clock.hpp"

class TimerManager {
public:
	using key_t = u64;
private:
	struct Function {
		using Callback = std::function<bool(double)>;

		Clock clock;

		bool toRemove = false;
		bool paused = false;

		double timer;
		double time;

		double error = 0.;

		Callback f;

		Function() {};
		Function(double timer, const Callback& f)
			: clock(timer), timer(timer), time(timer), f(f) {
		};
		Function(double timer, const Callback&& f)
			: clock(timer), timer(timer), time(timer), f(f) {
		};
	};

	static std::unordered_map<
		key_t, 
		std::shared_ptr<Function>
	> _functions;

public:

	static u32 getNFunction();

	static key_t addFunction(double timer, const Function::Callback&& f);
	static key_t addFunction(double timer, const Function::Callback& f);
	static void resetTimerFunction(key_t key);
	static void restartFunction(key_t key);
	static void removeFunction(key_t key);
	static void resumeFunction(key_t key);
	static void pauseFunction(key_t key);
	static void callFunction(key_t key);

	static bool functionsExist(key_t key);

	//ASSUREZ VOUS QUE LE POINTEUR RESTE VALIDE TOUT AU LONG DU PROCESSUS !!!!!!
	template<typename T = double>
	static key_t addPowerEase(double t, T* v, T min, T max, float p);
	template<typename T = double>
	static key_t addPowerOEase(double t, T* v, T min, T max, float p);
	template<typename T = double>
	static key_t addPowerIOEase(double t, T* v, T min, T max, float p);
	template<typename T = double>
	static key_t addCustomEase(double t, T* v, T min, T max, std::function<double(double)> f);
	template<typename T = double>
	static key_t addLinearEase(double t, T* v, T min, T max) {return TimerManager::addPowerEase<T>(t, v, min, max, 1);}
	template<typename T = double>
	static key_t addSquaredEase(double t, T* v, T min, T max){return TimerManager::addPowerEase<T>(t, v, min, max, 2);}
	template<typename T = double>
	static key_t addSquaredIOEase(double t, T* v, T min, T max){return TimerManager::addPowerIOEase<T>(t, v, min, max, 2);}
	template<typename T = double>
	static key_t addSinEase(double t, T* v, T min, T max);
	template<typename T = double>
	static key_t addSinOEase(double t, T* v, T min, T max);
	template<typename T = double>
	static key_t addSinIOEase(double t, T* v, T min, T max);

	static void update(double dt);
	static void updateInc(double dt, uint32_t n);
};

template<typename T>
TimerManager::key_t TimerManager::addPowerEase(double t, T* v, T min, T max, float p) {
	return addCustomEase<T>(t, v, min, max, [p](double x)->double {return pow(x, p);});
}
template<typename T>
TimerManager::key_t TimerManager::addPowerOEase(double t, T* v, T min, T max, float p) {
	return addCustomEase<T>(t, v, min, max, [p](double x)->double {return 1 - pow(1-x, p)});
}
template<typename T>
TimerManager::key_t TimerManager::addPowerIOEase(double t, T* v, T min, T max, float p) {
	return addCustomEase<T>(t, v, min, max, [p](double x)->double {
		return x < 0.5 ? pow(2*x, p) / 2. : 1 - pow(2 * (1 - x), p) / 2.;
	});
}
template<typename T>
TimerManager::key_t TimerManager::addSinEase(double t, T* V, T mmin, T max) {
	return TimerManager::addCustomEase<T>(t, v, min, max, [](double x)->double {
		return 1 - sin(C::PI * (1 - x) / 2.);
	});
}
template<typename T>
TimerManager::key_t TimerManager::addSinOEase(double t, T* V, T mmin, T max) {
	return TimerManager::addCustomEase<T>(t, v, min, max, [](double x)->double {
		return sin(C::PI * x / 2.);
	});
}
template<typename T>
TimerManager::key_t TimerManager::addSinIOEase(double t, T* V, T mmin, T max) {
	return TimerManager::addCustomEase<T>(t, v, min, max, [](double x)->double {
		return (x < 0.5 ? (1 - sin(C::PI * (1 - 2 * x) / 2.f)) : sin((x - 0.5) * C::PI)) / 2.f;
	});
}
template<typename T>
TimerManager::key_t TimerManager::addCustomEase(double t, T* v, T min, T max, std::function<double(double)> pf) {
	std::shared_ptr<Clock> clock = std::make_shared<Clock>(t);
	auto f = [t, v, min, max, pf, clock, intT = 0.f](double)mutable->bool {
		*v = static_cast<T>(min + (max - min) * pf(clock->elapsed() / t));
		if(clock->isOver()) {
			*v = max;
			return true;
		}
		return false;
	};
	return TimerManager::addFunction(.0, f);
}

using TM = TimerManager;