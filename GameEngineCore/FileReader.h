#ifndef G_FILE_READER
#define G_FILE_READER

#include<vector>
#include <string>
namespace te
{
	class FileReader
	{
	public:
		static std::vector<char> readFile(const std::string& filename);
	};
}

#endif // !G_FILE_READER

