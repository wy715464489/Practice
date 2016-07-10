#pragma once

//*****************************************************************************
// Method Interface
// Virtual interface to a method... allows non-templated callbacks
// (need one for each supported parameter count)
//*****************************************************************************

// base
class MethodInterface 
{
	public: virtual ~MethodInterface() {}
	virtual void *getObj() = 0; 
};

// no params
template<class RetVal>
class MethodInterface0 : public MethodInterface 
{
	public: virtual RetVal execute() = 0; 
};

// 1 param
template<class RetVal, class Param1>
class MethodInterface1 : public MethodInterface  
{
	public: virtual RetVal execute(Param1) = 0; 
};

// 2 params
template<class RetVal, class Param1, class Param2>
class MethodInterface2 : public MethodInterface
{
	public: virtual RetVal execute(Param1, Param2) = 0; 
};

// 3 params
template<class RetVal, class Param1, class Param2, class Param3>
class MethodInterface3 : public MethodInterface
{
	public: virtual RetVal execute(Param1, Param2, Param3) = 0; 
};

//*****************************************************************************
// Method
// templated callback... stores object and method used for callbacks
// (need one for each supported parameter count)
//*****************************************************************************

//*****************************************************************************
// no params
//*****************************************************************************
template<class T, class RetVal>
class Method0 : public MethodInterface0<RetVal>
{
public:
	Method0(T *pObj, RetVal (T:: *pMethod)()) : mpObj(pObj), mpMethod(pMethod) {}
	virtual void *getObj()		{ return mpObj; }
	virtual RetVal execute()	{ return (*mpObj.*mpMethod)(); }
private:
	T		*mpObj;
	RetVal	(T::*mpMethod)();
};

//*****************************************************************************
// 1 param
//*****************************************************************************
template<class T, class RetVal, class Param1>
class Method1 : public MethodInterface1<RetVal, Param1>
{
public:
	Method1(T *pObj, RetVal (T:: *pMethod)(Param1)) : mpObj(pObj), mpMethod(pMethod) {}
	virtual void *getObj()					{ return mpObj; }
	virtual RetVal execute(Param1 param1)	{ return (*mpObj.*mpMethod)(param1); }
private:
	T		*mpObj;
	RetVal	(T::*mpMethod)(Param1);
};

//*****************************************************************************
// 2 params
//*****************************************************************************
template<class T, class RetVal, class Param1, class Param2>
class Method2 : public MethodInterface2<RetVal, Param1, Param2>
{
public:
	Method2(T *pObj, RetVal (T:: *pMethod)(Param1, Param2)) : mpObj(pObj), mpMethod(pMethod) {}
	virtual void *getObj()									{ return mpObj; }
	virtual RetVal execute(Param1 param1, Param2 param2)	{ return (*mpObj.*mpMethod)(param1, param2); }
private:
	T		*mpObj;
	RetVal	(T::*mpMethod)(Param1, Param2);
};

//*****************************************************************************
// 3 params
//*****************************************************************************
template<class T, class RetVal, class Param1, class Param2, class Param3>
class Method3 : public MethodInterface3<RetVal, Param1, Param2, Param3>
{
public:
	Method3(T *pObj, RetVal (T:: *pMethod)(Param1, Param2, Param3)) : mpObj(pObj), mpMethod(pMethod) {}
	virtual void *getObj()												{ return mpObj; }
	virtual RetVal execute(Param1 param1, Param2 param2, Param3 param3)	{ return (*mpObj.*mpMethod)(param1, param2, param3); }
private:
	T		*mpObj;
	RetVal	(T::*mpMethod)(Param1, Param2, Param3);
};



