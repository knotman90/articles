#include <iostream>

	
template<class T, class U>
struct isConvertible {
	
	using Big = struct {char dummy[2];} ;
	using Small = char;

	constexpr static Small isConvertible_worker( U );
	constexpr static Big isConvertible_worker(...);
	//we use this dummy function because T might have a default constructor 
	//deleted or private. MakeT is only used by the compiler to evaluate sizeof()
	constexpr static  T makeT()  ;

	const static bool value = 
		sizeof(isConvertible_worker(makeT())) == sizeof(Small);
};


//const T* is convertible to const U* only if
//1) T and U are the same type
//2) U is public base of T
//3) U is void (everyting is convertible to a void pointer)
//the following return true only for the 1) and 2)
template <class T, class U>
struct isSubclassOf{
	constexpr static bool value = 
	isConvertible< const T*, const  U*>::value && !std::is_same<U,void>::value ;
};

//if T=U return false
template <class T, class U>
struct isSubclassOf_strict{
	constexpr static bool value = isSubclassOf<T,U>::value && !std::is_same<T,U>::value; 

};

struct TT{
	double A[4];
};


struct YY : TT {
	double A[4];
	char b;
};


struct ZZ  {
	double A[4];
};

int main(){
	
	using std::cout;
	using std::endl;
	// YY is convertible to TT since YY is a TT 
	cout<<isConvertible<YY, TT>::value<<endl;
	// TT is not convertible to YY since TT is smaller than YY
	cout<<isConvertible<TT, YY>::value<<endl;
	// ZZ is not convertible to TT even if they have the same size
	cout<<isConvertible<ZZ, TT>::value<<endl;

	cout<<isSubclassOf<TT,YY>::value<<endl;
	cout<<isSubclassOf<YY,TT>::value<<endl;

	cout<<isSubclassOf<ZZ,void>::value<<endl;

	cout<<isSubclassOf<ZZ,ZZ>::value<<endl;
	cout<<isSubclassOf_strict<ZZ,ZZ>::value<<endl;
	cout<<isSubclassOf_strict<const YY,TT>::value<<endl;

	cout<<isConvertible< int*, const int*>::value<<endl;

	return 0;
}