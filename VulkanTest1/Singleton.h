#ifndef GE_SINGLETON
#define GE_SINGLETON

namespace te
{

	template <typename T>
	class Singleton
	{
	protected:

		Singleton() {}
		inline static T* instance = nullptr;
	public:

		static bool isInitialized() {
			return instance != nullptr;
		};

		/**
		 * Одиночки не должны быть клонируемыми.
		 */
		Singleton<T>(Singleton<T>&) = delete;
		Singleton<T>(Singleton<T>&&) = delete;
		void operator=(const Singleton<T>&) = delete;

		static T* getInstance() {

			return instance;
		};

	};
}
#endif // !GE_SINGLETON


