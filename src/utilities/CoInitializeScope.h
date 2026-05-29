#pragma once

namespace meshproc
{
	namespace utilities
	{
		class CoInitializeScope
		{
		public:
			CoInitializeScope();
			~CoInitializeScope();
		};
	}
}
