using namespace std;

struct coordinates {
	double X;
	double Y;
	double Z;
};

template <typename T, int N> char (&myarray(T(&)[N]))[N];
