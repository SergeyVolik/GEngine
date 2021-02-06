#ifndef GE_TIME
#define GE_TIME
#include <chrono>

namespace te
{
	class Time
	{
	private:
		inline static std::chrono::steady_clock::time_point current = std::chrono::high_resolution_clock::now();
		inline static std::chrono::steady_clock::time_point prevTime = std::chrono::high_resolution_clock::now();

		inline static float delta = 0.0f;
	public:
		static unsigned int currentFrame;

	
		static void calcDelta()
		{
			
			if (currentFrame != 0)
			{
				prevTime = current;
			}

			current = std::chrono::high_resolution_clock::now();

			if (currentFrame != 0)
			{
				delta = std::chrono::duration<float, std::chrono::seconds::period>(current - prevTime).count();
			}

			currentFrame++;
			
		}

		static float getDelta()
		{
			return delta;
		}
	};
}
#endif // !GE_TIME


