
//*****************************************************************************
template<class T>
void FSM::State::setEnterMethod(T *pObj, void (T::*method)())
{
	mpEnterMethod = new Method0<T, void>(pObj, method);
}

//*****************************************************************************
template<class T>
void FSM::State::setExitMethod(T *pObj, void (T::*method)())
{
	mpExitMethod = new Method0<T, void>(pObj, method);
}

//*****************************************************************************
template<class T>
void FSM::State::setTickMethod(T *pObj, void (T::*method)(float fdt))
{	
	mpTickMethod = new Method1<T, void, float>(pObj, method);
}
