#include "Triangle.h"

using namespace meshproc;
using namespace meshproc::data;

Triangle::Triangle(uint32_t i0, uint32_t i1, uint32_t i2)
{
	m_idx[0] = i0;
	m_idx[1] = i1;
	m_idx[2] = i2;
}
