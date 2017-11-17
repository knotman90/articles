# How polymorphis is implemented
Polymorphis is the ability of assigning a pointer to  a base class to  a derived class  class object.
When a method is invoked on the pointer, the derived implementation (if present) of the method is called, otherwise the inherited method is.
It is implemented with a cost in term of memory and time. For each class a virtual method table is stored and a pointer to it is added to the definition (transparently  by the compiler) of each class containing a virtual method (or deriving from a class that contains a virtual method).  The table in turn contains pointer to the actual implementation of the virtual methods for the derived class. So the compiler only knows the object through a pointer to its base class, but it can still generate the correct code since it can indirect the calls to overridden methods  via the virtual method table, then lookup for the method in the table and finally call it. So polymorphism comes at a cost of storing a virtual method table for each class, a pointer to it in each instance of a polymorphic class and two level in indirection when calling a virtual method.
Another pitfall is that since the indirection is required, usually virtual methods cannot be in-lined.

#Curious Recurring Template Pattern
**Key idea is: polymorphism without extra run-time cost. Static polymorphism**
Templates can mitigated those problems via the so called static polymorphism, or simulated dynamic binding. 
The idea is to inject the base class with information at compile time about the Derived class as in the following example: 
```c++
template <typename Child>
struct Base{
	void interface(){
		static_cast<Child*>(this)->implementation();
	}
};

struct Derived : Base<Derived>{
void implementation(){
        cerr << "Derived implementation\n";
    }
};
int main(){
    Derived d;
    d.interface();  // Prints "Derived implementation"
}
```
This is possibly the simplest example of static polymorphism possible. 
The base class offers a functionality that depends on a parameter. This parameter is the type of the class that need this functionality. The base class tails the functionality for the Derived class.

`Base` is a class that takes a template parameter: `Derived` which is used in a `static_cast` construct. Is uses the information about the type of `Derived` in order to perform a cast at compile time. `Derived` is a class that inherits from `Base<Derived>` meaning that `Derived` is also of type `Base<Derived>` hence inheriting the `interface` method. Since interface is called from an object of type `Derived` is it safe to cast `this` to `Derived`.

As an example the following is what `Derived` looks showing also  the inherited method from `Base<Derived>`.
```c++
struct Derived{
	void interface(){
		static_cast<Derived*>(this)->implementation();
	}
	void implementation(){
		cerr << "Derived implementation\n";
	}
};
```
It is clear that there is nothing strange or funny in the definition of `Derived`, neither of the `interface()` method. 


As a more complex and usefule example, consider that scenario in which we want to add the comparison functionality to a certain set of objects, still, decoupling the code implementing that functionality from the classes themself and possibly reducing code duplication i.e. writing the logic of the comparison methods only once  and using some knowledge about a common interface that all the object sharing the functionality exposes. The key point for this specific example is that all object that want to have the *Comparable* functionality need to expose the `<` operator or an equivalent method. This is all we need from within an object.

In the usual way using polymorphism one can do as follows:
```c++
struct Comparable {
  virtual bool less(const Comparable* p1) const = 0;
};

bool equals(Comparable* p1, Comparable* p2) {
  return !(p1->less(p2)) && !(p2->less(p1));
}
bool notEquals(Comparable* p1, Comparable* p2) {
  return !(equals(p2,p2));
}
struct Person : public Comparable {
  int age;
  Person(const int i) : age(i){};
  Person() = default;
  virtual bool less(const Comparable* p) const {
    return age < (static_cast<Person*>(p))->age;
  }
};
```
Armed with this, one can get equals and `notEquals` methods for free for any object that uses the `Comparable` interface and provides the `less` compasison method only. This approach uses `virtual` methods and se have seen, it comes with a cost attached to it.

Let's use CRTP to obtain the same effect, at compile time!

```c++
template <class Derived>
struct ComparableT {};

template <class Derived>
inline bool operator==(const ComparableT<Derived>& o1,
                const ComparableT<Derived>& o2) {
  const Derived& d1 = static_cast<const Derived&>(o1);
  const Derived& d2 = static_cast<const Derived&>(o2);
  return !(d1 < d2) && !(d2 < d1);
}
template <class Derived>
inline bool operator!=(const ComparableT<Derived>& o1,
                const ComparableT<Derived>& o2) {
  return !(o1 == o2);
}

struct Person_T : ComparableT<Person_T> {
  Person_T() = default;
  Person_T(const int i) : age(i) {};
  Person_T(const ComparableT<Person_T>&){};
  int age;
  bool operator<(const Person_T& o1) const { return age < o1.age; }
};
```
Now we can make an object have `==` and `!=` operator by simply implementing the `<` operator and using the CRTP as in the following:

```c++
class Person : Comparable<Person>{
	bool operator<(Person p1, Person p2){
		return p1.age < p2.age;
	}
}
```

`operator==` and `operator!=` assumes that:

1.  the type `Derived` of the object passed as parameter have `ComparableT<Derived>` as superclass.
2.  `Derived` exposed the `operator<` method.

When `operator==` is called on two `Person_T` objects, the calls look like the following:

```c++
bool operator==(const Comparable<Person_T>& o1,  Comparable<Person_T>& o2) {
  const Person_T& d1 = static_cast<const PersonT&>(o1);
  const Person_T& d2 = static_cast<const PersonT&>(o2);
  return !(d1 < d2) && !(d2 < d1);
}
```

It is safe to pass a `Person_T` when a `Comparable<Person_T>& ` because Person_T is subclass of `Comparable<Person_T>` **and** because the parameter are taken by reference (All references/pointers to derived types are converted implicitly to base objects references/pointers when necessary).
Plus, we can safely downcast  from  `Comparable<Person_T>& ` to `  const Person_T& ` as the actual parameter passed to the `operator==` is a const reference to Person_T.
Thus when  the compiler encounters `d1 < d2` can simply emit the correct object code since at this stage it knows the definition of the `operator<` for Person_T.
*Comparison* is quite an orthogonal functional that we have succesfully distilled out of a specific class without any run-time cost.
----------------------- ------------------------------------
**Note**
that for instance the following would be horribly wrong:
```c++
template <class Derived>
inline bool operator==(const ComparableT<Derived> o1,
                const ComparableT<Derived> o2) {
  const Derived& d1 = *static_cast<const Derived*>(&o1);
  const Derived& d2 = *static_cast<const Derived*>(&o2);
  return !(d1 < d2) && !(d2 < d1);
}
```

since `o1` and `o2` would be sliced (via object slicing mechanism) and, moreover they would be new object with no information about the additional methods and data of a `Person_T`. One can still `static_cast` `o1` to a a `Derived&` but the result is **undefined behaviour**.
----------------------- ------------------------------------

To give you an idea of the differences in performance the following is a super simple uses the two approaches described above to compare Persons.
 
- The following is used to test the polymorphic version of `Person`
```c++
void testPoly(const int size) {
  vector<Comparable*> coll;
  coll.reserve(size);
  for (int i = 0; i < size; i++) {
    coll.push_back(new Person(i));
  }
  clock_t begin = clock();
  int count = 0;
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++) count += equals(coll[i], coll[j]);

  clock_t end = clock();
  double elapsed_time = double(end - begin) / (CLOCKS_PER_SEC / 1000.0);

  cout << __PRETTY_FUNCTION__<<" "<<count << ":  "<<elapsed_time<<endl;

  for (int i = 0; i < size; i++) {
    delete coll[i];
  }
```

- While the following is used to test the templated version:
```c++
		void testTemplate(const int size) {

		  vector<Person_T> coll;
		  coll.reserve(size);
		  for (int i = 0; i < size; i++)
		    coll.push_back(Person_T(i));
		    
		  clock_t begin = clock();
		  int count = 0;
		  for (int i = 0; i < size; i++)
		    for (int j = 0; j < size; j++) count += coll[i]== coll[j];

		  clock_t end = clock();
		  double elapsed_time = double(end - begin) / (CLOCKS_PER_SEC / 1000.0);

		  cout << __PRETTY_FUNCTION__<<" "<< count << ":  "<<elapsed_time<<endl;

		}
```
The following table shows execution time for the afortmentioned functions when the `size=20000`.  
The processor is   a `Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz`

## g++ (GCC) 7.2.0
| Optimization Level | virtual functions (ms)  |   CRTP(*ms*) |
| ------------- |:-------------:| -----:|
|`-O0`      | 3726.49 | 3785.52|
| `-O1`      | 1481.76 | 234.984 |
| `-O2`     | 1312.49 | 211.953 |
| `-O3`     | 846.731 | 56.939 |


## clang version 5.0.0
| Optimization Level | virtual functions (ms)  |   CRTP(*ms*) |
| ------------- |:-------------:| -----:|
|`-O0`      | 3772.2 | 3688.67|
| `-O1`      | 2241.17 | 2132.47 |
| `-O2`     | 913.161| 28.567 |
| `-O3`     | 1036.47 | 28.593 |


![](http://i65.tinypic.com/rvew0h.png)  "Logo Title Text 1")
It is clear that the templated, compile time polimorphism has an important performance advantage.



**Plus note that the size of `Person` and `Person_T` differs as`sizeof(Person)=16` while `sizeof(Person)=4`**.

________________________________________________________________________
**Continua qui con altra roba**

Another example could be the the following.
```c++
template <class T>
class Printer{
	Printer* print(std::string a){
		cout<<a;
		return static_cast<T*>(this);
	}
};

class PrintTwicePrinter : Printer<CoolPrinter>{
	CoolPrinter* anotherPrintMethod(){
		//do stuff
		
		return this;
	}
};
```
It is possible to chain methods in the following way:
```c++
CoolPrinter p;
p.print().anotherPrintMethod();
```

if CTRP non fosse usate allora print nella classe base ritornerebbe semplicemente un puntatore alla base e quindi dopo la chiamata a `print()` il compilatore non saprebbe come chiamare il metodo `anotherPrintMethod()`. IUnvece cosi il metodo della classe base sa a compile time che tipo di oggetto deve ritornare e quindi la chiamata a print ritorna il tipo della classe derivata corretta.












