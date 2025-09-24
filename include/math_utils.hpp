#include <iostream>
#include <stdexcept>

/**
 * @brief 线性映射函数，将数值从初始区间映射到目标区间
 * 
 * @param value 要映射的数值
 * @param from_min 初始区间最小值
 * @param from_max 初始区间最大值
 * @param to_min 目标区间最小值
 * @param to_max 目标区间最大值
 * @return double 映射后的值
 * 
 * @throws std::invalid_argument 如果初始区间长度为0（即 from_min == from_max）
 */
float LinearMap(float value, float from_min, float from_max, float to_min, float to_max) ;