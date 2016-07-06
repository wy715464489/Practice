//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Finite state machine inline functionality
// 
//*****************************************************************************


//*****************************************************************************
template<class T>
void VuFSM::VuState::setEnterMethod(T *pObj, void (T::*method)())
{
	VUASSERT(mpEnterMethod == VUNULL, "Enter method already exists");
	mpEnterMethod = new VuMethod0<T, void>(pObj, method);
}

//*****************************************************************************
template<class T>
void VuFSM::VuState::setExitMethod(T *pObj, void (T::*method)())
{
	VUASSERT(mpExitMethod == VUNULL, "Exit method already exists");
	mpExitMethod = new VuMethod0<T, void>(pObj, method);
}

//*****************************************************************************
template<class T>
void VuFSM::VuState::setTickMethod(T *pObj, void (T::*method)(float fdt))
{
	VUASSERT(mpTickMethod == VUNULL, "Tick method already exists");
	mpTickMethod = new VuMethod1<T, void, float>(pObj, method);
}

//*****************************************************************************
template<class T>
void VuFSM::VuState::setDrawMethod(T *pObj, void (T::*method)())
{
	VUASSERT(mpDrawMethod == VUNULL, "Draw method already exists");
	mpDrawMethod = new VuMethod0<T, void>(pObj, method);
}