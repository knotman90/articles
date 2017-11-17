#Strict Aliasing

#####Short versions:
You cannot have two pointers with different types refer to the same memory location because compilers assumes that it never happens.

```c++
int UB(uint32_t const* data, const int count){
	uint32_t val=0;
	for(int i=0;i<count;i++){
		val+=*(static_cast<uint8_t*>(data)+i);
	}
}
```

You cannot have two pointers to an aggregate structure or union refer to the same memory location even if they have the exact same content or layout in memory.

```c++
struct A{
	int a;
	short b;
	double c;
};
struct B{
	int a;
	short b;
	double c;
};
 A aobj;
//illegal!
 B* = &aobj;
 
 //legal since A and C are the same object afterall
typedef  A C;
  C* c= &aobj;
```

#####Long story:
The **strict aliasing** is an assumpion made by the compilers mostly for optimization purposes that two pointers with different types do not alias each other i.e. they do not refer to the same memory location.
There are cases where this is doable, i.e. when types are **compatible**. Compatible types are such that they only differ because of the **sign** of **qualifier**
You can cast a `uint*` to an `int*` or a `int * volatile` to a `int* const`for instance.

Why is that useful?
Well, consider a simple scenario like the following:

```c++
void aliasingOptimization(uint32_t* a, uint16_t* b, const int count){
	for(int i=0;i<count;i++){
		a[i] = *static_cast<uint32_t*>(b);
	}
}
```

Now imagine that `a` and `b` overlaps. What happen when `b` gets overwrittern? `b` changes too! the only way for the compiler to enforce this change and make sure that any subsequent iteration is correct is to **load b at each iteration**. If you instead apply the strict aliasing rule, can assume that `a` and `b` never overlap and hence **load b only once**!
