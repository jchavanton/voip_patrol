#include <algorithm>


namespace vp {

	void tolower(string s) {
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	}


}
