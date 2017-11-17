#include<iostream>
#include<array>
#include<type_traits>
#include <iomanip>      // std::setprecision
#include <math.h>
using std::cout;
using std::endl;

//CTRP
template<class T>
struct Printable{
	void print() const {
		(static_cast<const T*>(this))->print();
	}
};

template<int n, class T = long long >
struct Factorial{
	static constexpr T result = n* Factorial<n-1>::result;
	
};

template<class T>
struct Factorial<0,T> {
	static constexpr T result = 1;	
};

//Fibonacci
template<int n, class T = long long>
struct Fibonacci : public Printable<Factorial<n,T>>{
	static constexpr T result = Fibonacci<n-1>::result + Fibonacci<n-2>::result;
	static constexpr 
	void print(){
		cout<<"Fibonacci "<<n<<"="<<result<<endl;
	}
};

template<class T>
struct Fibonacci<0,T> : public Printable<Factorial<0,T>> {
	static constexpr T result = 0;
};
template<class T>
struct Fibonacci<1,T> : public Printable<Factorial<1,T>>{
	static constexpr T result = 1;
};


//3.1415926535897932384626

//compile time fractions
template<class Type, Type N, Type D>
struct Fraction : public Printable<Fraction<Type,N,D>>{
	typedef Type BaseType;
	static constexpr Type Num = N;
	static constexpr Type Den = D;

	static constexpr 
	void print(){
		cout<<N<<"/"<<D<<endl;
	}
};

template<class F, typename F::BaseType mult>
struct FractionScalarMultiplication{
	typedef Fraction<typename F::BaseType,mult*F::Num,F::Den> result; 
};


template<class GCD>
struct PrintableGCD : Printable<GCD>{
	static constexpr 
	void print(){
		cout<<"GCD("<<GCD::N<<","<<GCD::M<<")="<<GCD::result<<endl;
	}
};
template<class F,  F A, F B, typename = void>
struct GCD : PrintableGCD<GCD<F,A,B>>{
	typedef F BaseType;
	constexpr static F result = GCD<F , B , A%B>::result;
	constexpr static F N = A;
	constexpr static F M = B;	
};


template<class F, F A , F B >
struct GCD<F,A,B, typename std::enable_if<B==0>::type> : PrintableGCD<GCD<F,A,B, typename std::enable_if<B==0>::type>>{
	constexpr static F result = A;
	constexpr static F N = A;
	constexpr static F M = B;
	
};

template<class Frac>
struct FractionSimplify{
	constexpr static typename Frac::BaseType gcd = GCD<typename Frac::BaseType, Frac::Num, Frac::Den>::result;
	typedef Fraction<typename Frac::BaseType, Frac::Num/gcd, Frac::Den/gcd> result;
};

template<class Op1, class Op2, 
		typename = typename std::enable_if<
			std::is_same<typename Op1::BaseType,typename Op2::BaseType>::value>::type
		>
struct FractionMultiplication{
	typedef Fraction<typename Op1::BaseType, Op1::Num*Op2::Num, Op1::Den*Op2::Den> result;
	typedef typename FractionSimplify<result>::result result_simplified;
};

template<class Op1, class Op2, 
		typename = typename std::enable_if<
			std::is_same<typename Op1::BaseType,typename Op2::BaseType>::value>::type
		>
struct FractionDivision{
	typedef Fraction<typename Op1::BaseType, Op1::Num*Op2::Den, Op1::Den*Op2::Num> result;
	typedef typename FractionSimplify<result>::result result_simplified;
};

template< int k,  int n, bool Sign, class Type,
	typename = typename 
			std::enable_if<
				std::is_floating_point<Type>::value 
			>::type
	>
struct AlternateSumOddReciprocal{
	constexpr static Type sign(){
		if constexpr (Sign)
			return Type(-1);
		return Type(1);	
	} 
	constexpr static  Type value = sign()*Type(1)/(k) +
				AlternateSumOddReciprocal<k+2,n-2,!Sign, Type>::value;
};

template< int k, bool Sign, class Type>
struct AlternateSumOddReciprocal<k,0,Sign,Type>{
	constexpr static Type value = Type(0);
};


//Extremely slow convergence!
//-------PI using Gregory formula------
template< int n, class Type = double,
	typename = typename 
			std::enable_if<
				std::is_floating_point<Type>::value
			>::type
	>
struct PI{
	constexpr static  Type value = 4*(1 +
							AlternateSumOddReciprocal<3,n,true, Type>::value);
};


//-------PI using Newton formula--------
template< typename Type, int num, int den, int k>
struct _PI_Netwton_worker{
	constexpr static Type value = 1 + (Type(num)/Type(den))*
		_PI_Netwton_worker<Type,num+1,den+2,k-1>::value;
};

template<typename Type, int num, int den>
struct _PI_Netwton_worker<Type,num,den,0>{
	constexpr static Type value = 1;
};

template<int k, class Type=double,
		typename =  typename std::enable_if< 0 < k >::type
		>
struct PI_Netwton{
	constexpr static Type value =  Type(2)*_PI_Netwton_worker<Type,1,3,k>::value;
};


template<int val>
struct Int{
	constexpr static int value = val;
};

template<int val>
struct Short{
	constexpr static short value = val;
};

template<typename T, T val>
struct TVal{
	constexpr static T value = val;
};




template<typename H, typename T = void>
struct List{
	typedef H head;
	typedef T tail;
};

template<typename List>
struct ListLength{
	constexpr static int value = 1 + ListLength<typename List::tail>::value;
};
template<>
struct ListLength<void>{
	constexpr static int value = 0;
};

template<class L, int k>
struct get_element{
	constexpr static auto value = get_element<typename L::tail,k-1>::value;
};
template<class L>
struct get_element<L,0>{
	constexpr static auto value = L::head::value;
};

template<class Element, class Lst>
struct ListSearch{
	constexpr static bool search(){
		if constexpr(std::is_same<Element,typename Lst::head>::value)
			return true;
		if constexpr(std::is_void< typename Lst::tail>::value)
			return false;
		else
			return ListSearch< Element, typename Lst::tail>::search();
	}
	constexpr  auto operator()(){
		return search();
	}
};

template<class Element, class Lst = void>
struct ListPreprend{
	typedef List<Element, Lst> result;
};

template<class Element, class Lst>
struct ListAppend{
	//since this is not the end of the list, add Element in the tail recursively
	typedef typename ListAppend<Element,typename Lst::tail>::result new_tail;
	typedef List<typename Lst::head,  new_tail > result;
};
template<class Element>
struct ListAppend<Element,void>{
	typedef List<Element, void> result;
};

template<class Element, class Lst, typename = typename std::enable_if< ListSearch<Element, Lst>::search()>::type >
struct ListSearchIndex{
	constexpr static unsigned int search_index(){
		if constexpr(std::is_same<Element,typename Lst::head>::value)
			return 0;
		else
			return 1+ListSearchIndex< Element, typename Lst::tail>::search_index();

	}
	constexpr auto operator()(){
		return search_index();
	} 
};


template<long N, long D>
using LFraction=Fraction<long, N,D> ;

int main(){
	
	typedef 
		List
		< 
			TVal<unsigned int,-1>, 
			List
			<
				TVal<long,7046>,
				List
				<
					TVal<short,12378>,
					List
					<
						TVal<long long,14545434234254>
					>
				>
			>
		> list;
			
	//test search and list preprend
	cout<<"Search Prepend = "<<ListSearch<TVal<unsigned int,123>,list>::search()<<endl;

	typedef ListPreprend<TVal<unsigned int,123>,list>::result newlist;
	cout<<"Search Prepend = "<<ListSearch<TVal<unsigned int,123>,newlist>::search()<<endl;
	cout<<"SearchIndex Prepend = "<<ListSearchIndex<TVal<unsigned int,123>,newlist>::search_index()<<endl;
// fails to compile because the node to be searched is not in thelist
	//cout<<"SearchIndex Prepend = "<<ListSearchIndex<TVal<unsigned int,1238787>,newlist>::search_index()<<endl;

	//test search and list append
	cout<<"Search Append = "<<ListSearch<TVal<unsigned int,321>,list>::search()<<endl;
	typedef ListAppend<TVal<unsigned int,321>,list>::result newlist_append;
	cout<<"Search Append = "<<ListSearch<TVal<unsigned int,321>,newlist_append>::search()<<endl;
	cout<<"SearchIndex Append = "<<ListLength<newlist_append>::value<<" "<<ListSearchIndex<TVal<long long,14545434234254>,newlist_append>::search_index()<<endl;


		
	cout<<ListLength<list>::value<<endl;
	cout<<"value "<<get_element<list,0>::value<<endl;
	cout<<"value "<<get_element<list,1>::value<<endl;
	cout<<"value "<<get_element<list,2>::value<<endl;
	cout<<"value "<<get_element<list,3>::value<<endl;


	constexpr int n = 15;
	
	Fibonacci<20>::print();
		
	typedef LFraction<4,5> four5;
	four5::print();
	typedef FractionScalarMultiplication<four5, 2>::result eight5;
	eight5::print();

	GCD<long,8,64>::print();


	typedef LFraction<20,6> twenty6;
	typedef FractionSimplify<twenty6>::result simplifiedTwenty5;
	simplifiedTwenty5::print();

	FractionMultiplication<four5,twenty6>::result::print();
	FractionMultiplication<four5,twenty6>::result_simplified::print();
	
	FractionDivision<four5,twenty6>::result::print();
	FractionDivision<four5,twenty6>::result_simplified::print();
	
	constexpr int LIM = 500;
	cout<< std::setprecision(10)<<PI<LIM>::value<<" "<<M_PI<<"- err = "<<std::fabs(PI<LIM>::value-M_PI)<<endl;
	
	//cout<<PI_Netwton<20>::value<<endl;
	cout<< std::setprecision(10)<<PI_Netwton<LIM,long double>::value<<" "<<M_PI<<"- err = "<<std::fabs(PI_Netwton<LIM,long double>::value-M_PI)<<endl;

}