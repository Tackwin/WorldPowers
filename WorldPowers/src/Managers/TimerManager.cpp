#include "TimerManager.hpp"

#include <cassert>
#include <utility>
#include <string>

#include "./../Time/Clock.hpp"

std::unordered_map<
	TM::key_t,
	std::shared_ptr<TM::Function>
> TimerManager::_functions;

u32 TimerManager::getNFunction() {
	return _functions.size();
}

TM::key_t TimerManager::addFunction(double timer, const Function::Callback&& f) {
	//auto k = static_cast<key_t>(uuid());
	auto k = 0;

	_functions[k] = std::make_shared<Function>(timer, std::forward<const Function::Callback>(f));
	return k;
}

TM::key_t TimerManager::addFunction(double timer, const Function::Callback& f) {
	//auto k = static_cast<key_t>(uuid());
	auto k = 0;

	_functions[k] = std::make_shared<Function>(timer, f);
	return k;
}

void TimerManager::resetTimerFunction(TM::key_t key) {
	auto it = _functions.find(key);
	assert(it != std::end(_functions) && "Can't find the function");
	it->second->clock.reset();
	it->second->paused = false;
}
void TimerManager::restartFunction(TM::key_t key) {
	auto it = _functions.find(key);
	assert(it != std::end(_functions) && "Can't find the function");
	it->second->clock.restart();
	it->second->paused = false;
}
void TimerManager::callFunction(TM::key_t key) {
	auto it = _functions.find(key);
	assert(it != std::end(_functions) && "Can't find the function");
	it->second->toRemove = it->second->f(0);
}
bool TimerManager::functionsExist(TM::key_t key) {
	return _functions.find(key) != std::end(_functions);
}
void TimerManager::removeFunction(TM::key_t key) {
	auto it = _functions.find(key);
	assert(it != std::end(_functions) && "Can't find the function");
	it->second->toRemove = true;
}
void TimerManager::pauseFunction(TM::key_t key) {
	auto it = _functions.find(key);
	assert(it != std::end(_functions) && "Can't find the function");
	it->second->clock.pause();
	it->second->paused = true;
}
void TimerManager::resumeFunction(TM::key_t key) {
	auto it = _functions.find(key);
	assert(it != std::end(_functions) && "Can't find the function");
	it->second->clock.resume();
	it->second->paused = false;
}

void TimerManager::update(double dt) {
	for (auto it = std::begin(_functions); it != std::end(_functions);) {
		auto &f = it->second;
		if (f->toRemove) {
			_functions.erase((it++)->first);
			continue;
		}

		if (f->clock.isOver()) {

			if (f->clock.timer == 0.) {
				f->toRemove = f->f(dt);
			}
			else { // i should get rid of all these branching in the *MAIN* loop
				for (f->error += f->clock.elapsed(); !f->toRemove && f->clock.timer < f->error; f->error -= f->clock.timer) {
					f->toRemove = f->f(f->clock.elapsed());
				}
			}

			f->clock.restart();
		}

		++it;
	}
}

void TimerManager::updateInc(double dt, u32 n) {
	for (u32 i = 0u; i < n; ++i) {
		update(dt / n);
	}
}
