//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Method implementation
// 
//*****************************************************************************

#pragma once


//*****************************************************************************
// Method Interface
// Virtual interface to a method... allows non-templated callbacks
// (need one for each supported parameter count)
//*****************************************************************************

// base
class VuMethodInterface { public: virtual ~VuMethodInterface() {} virtual void *getObj() = 0; };

// no params
template<class RetVal>
class VuMethodInterface0 : public VuMethodInterface { public: virtual RetVal execute() = 0; };

// 1 param
template<class RetVal, class Param1>
class VuMethodInterface1 : public VuMethodInterface  { public: virtual RetVal execute(Param1) = 0; };

// 2 params
template<class RetVal, class Param1, class Param2>
class VuMethodInterface2 : public VuMethodInterface  { public: virtual RetVal execute(Param1, Param2) = 0; };

// 3 params
template<class RetVal, class Param1, class Param2, class Param3>
class VuMethodInterface3 : public VuMethodInterface  { public: virtual RetVal execute(Param1, Param2, Param3) = 0; };


//*****************************************************************************
// Method
// templated callback... stores object and method used for callbacks
// (need one for each supported parameter count)
//*****************************************************************************

//*****************************************************************************
// no params
//*****************************************************************************
template<class T, class RetVal>
class VuMethod0 : public VuMethodInterface0<RetVal>
{
public:
	VuMethod0(T *pObj, RetVal (T:: *pMethod)()) : mpObj(pObj), mpMethod(pMethod) {}
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
class VuMethod1 : public VuMethodInterface1<RetVal, Param1>
{
public:
	VuMethod1(T *pObj, RetVal (T:: *pMethod)(Param1)) : mpObj(pObj), mpMethod(pMethod) {}
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
class VuMethod2 : public VuMethodInterface2<RetVal, Param1, Param2>
{
public:
	VuMethod2(T *pObj, RetVal (T:: *pMethod)(Param1, Param2)) : mpObj(pObj), mpMethod(pMethod) {}
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
class VuMethod3 : public VuMethodInterface3<RetVal, Param1, Param2, Param3>
{
public:
	VuMethod3(T *pObj, RetVal (T:: *pMethod)(Param1, Param2, Param3)) : mpObj(pObj), mpMethod(pMethod) {}
	virtual void *getObj()												{ return mpObj; }
	virtual RetVal execute(Param1 param1, Param2 param2, Param3 param3)	{ return (*mpObj.*mpMethod)(param1, param2, param3); }
private:
	T		*mpObj;
	RetVal	(T::*mpMethod)(Param1, Param2, Param3);
};
