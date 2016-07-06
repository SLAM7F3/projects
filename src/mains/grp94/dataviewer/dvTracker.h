#ifndef DVTRACKER
#define DVTRACKER
#include <map>  // hash_map has an error! map will do
using namespace std;
class dvTracker{
public:
	typedef map<long, long> maptype;
	dvTracker(){}
	void touch(){list.clear();};
	void touch(long id){list.erase(id);};
	bool isTracked(long id)
	{
		maptype::iterator it=list.find(id);
		list[id]=0;
		return(it!=list.end());
	};
private:
	maptype list;
};
#endif
