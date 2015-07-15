#pragma once

template<typename T>
class DoubleBuffer
{
public:
	DoubleBuffer():current_index_(0){}
	void swap() { current_index_^=0x1; }
	T& front() { return t_[current_index_]; }
	T& back() { return t_[current_index_^1]; }
	T& get(size_t index) { return t_[index]; }
private:
	T t_[2];
	size_t current_index_;
};