# include "math_utils.hpp"

float LinearMap(float value, float from_min, float from_max,
                 float to_min, float to_max) {
    // 检查初始区间是否有效
    if (from_min == from_max) {
        throw std::invalid_argument("初始区间长度不能为0");
    }
    
    // 计算初始区间和目标区间的比例
    float from_range = from_max - from_min;
    float to_range = to_max - to_min;
    
    // 标准化数值到[0,1]范围
    float normalized = (value - from_min) / from_range;
    
    // 映射到目标区间
    return to_min + (normalized * to_range);
}