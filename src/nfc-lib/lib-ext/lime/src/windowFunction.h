#ifndef WINDOW_FUNCTION_GEN_H
#define WINDOW_FUNCTION_GEN_H

#include <vector>

void GenerateWindowCoefficients(int func, int fftsize, std::vector<float> &windowFcoefs, float amplitudeCorrection);

#endif