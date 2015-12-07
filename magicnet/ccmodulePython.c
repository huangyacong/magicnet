#include "ccmodulePython.h"


PyObject *MagicNetGateInit(PyObject *module, PyObject* args)
{
	bool bResult;
	char *pcLogName;
	int iTimeOut;
	unsigned short usMax;
	unsigned short usOutPort;
	unsigned short usInPort;

	if(!PyArg_ParseTuple(args, "siHHH", &pcLogName, &iTimeOut, &usMax, &usOutPort, &usInPort))
	return NULL;

	bResult = SeMagicNetSInit(&kMagicNetGate, pcLogName, iTimeOut, usMax, usOutPort, usInPort);
	return Py_BuildValue("O", bResult == true ? Py_True : Py_False);
}

PyObject *MagicNetGateFin(PyObject *module, PyObject* args)
{
	SeMagicNetSFin(&kMagicNetGate);
	return Py_BuildValue("O", Py_True);
}

PyObject *MagicNetGateProcess(PyObject *module, PyObject* args)
{
	SeMagicNetSProcess(&kMagicNetGate);
	return Py_BuildValue("O", Py_True);
}

PyObject *MagicNetSvrInit(PyObject *module, PyObject* args)
{
	bool bResult;
	char *pcLogName;
	int iTimeOut;
	unsigned short usInPort;

	if(!PyArg_ParseTuple(args, "siH", &pcLogName, &iTimeOut, &usInPort))
	return NULL;

	bResult = SeMagicNetCInit(&kMagicNetSvr, pcLogName, iTimeOut, usInPort);
	return Py_BuildValue("O", bResult == true ? Py_True : Py_False);
}

PyObject *MagicNetSvrFin(PyObject *module, PyObject* args)
{
	SeMagicNetCFin(&kMagicNetSvr);
	return Py_BuildValue("O", Py_True);
}

PyObject *MagicNetSvrReg(PyObject *module, PyObject* args)
{
	bool bResult;
	char *pcSvrName;

	if (!PyArg_ParseTuple(args, "s", &pcSvrName))
	return NULL;

	bResult = SeMagicNetCReg(&kMagicNetSvr, pcSvrName);
	return Py_BuildValue("O", bResult == true ? Py_True : Py_False);
}

PyObject *MagicNetSvrSendClient(PyObject *module, PyObject* args)
{
	bool bResult;
	HSOCKET kHSocket;
	const char *pcBuf;
	int iLen;

	if (!PyArg_ParseTuple(args, "Ks#", &kHSocket, &pcBuf, &iLen))
	return NULL;

	bResult = SeMagicNetCSendClient(&kMagicNetSvr, kHSocket, pcBuf, iLen);
	return Py_BuildValue("O", bResult == true ? Py_True : Py_False);
}

PyObject *MagicNetSvrBindClient(PyObject *module, PyObject* args)
{
	HSOCKET kHSocket;
	char *pcSvrName;
	
	if(!PyArg_ParseTuple(args, "Ks", &kHSocket, &pcSvrName))
	return NULL;

	SeMagicNetCBindClientToSvr(&kMagicNetSvr, kHSocket, pcSvrName);
	return Py_BuildValue("O", Py_True);
}

PyObject *MagicNetSvrCloseClient(PyObject *module, PyObject* args)
{
	HSOCKET kHSocket;

	if(!PyArg_ParseTuple(args, "K", &kHSocket))
	return NULL;

	SeMagicNetCCloseClient(&kMagicNetSvr, kHSocket);
	return Py_BuildValue("O", Py_True);
}

PyObject *MagicNetSvrSendSvr(PyObject *module, PyObject* args)
{	
	bool bResult;
	char *pcBuf;
	char *pcSvrName;
	int iLen;

	if(!PyArg_ParseTuple(args, "ss#", &pcSvrName, &pcBuf, &iLen))
	return NULL;

	bResult = SeMagicNetCSendSvr(&kMagicNetSvr, pcSvrName, pcBuf, iLen);
	return Py_BuildValue("O", bResult == true ? Py_True : Py_False);
}

PyObject *MagicNetSvrRead(PyObject *module, PyObject* args)
{
	enum MAGIC_STATE result;
	HSOCKET rkRecvHSocket;
	char *pcBuf;
	int riBufLen;
	
	rkRecvHSocket = 0;
	riBufLen = 0;
	pcBuf = 0;
	result = SeMagicNetCRead(&kMagicNetSvr, &rkRecvHSocket, &pcBuf, &riBufLen);
	return Py_BuildValue("iKs#", result, rkRecvHSocket, riBufLen > 0 ? pcBuf : "", riBufLen);
}

static void AddIntConstant(PyObject *module)
{
	PyModule_AddIntConstant(module, "MAGIC_SHUTDOWN_SVR", MAGIC_SHUTDOWN_SVR);
	PyModule_AddIntConstant(module, "MAGIC_IDLE_SVR_DATA", MAGIC_IDLE_SVR_DATA);
	PyModule_AddIntConstant(module, "MAGIC_CLIENT_CONNECT", MAGIC_CLIENT_CONNECT);
	PyModule_AddIntConstant(module, "MAGIC_CLIENT_DISCONNECT", MAGIC_CLIENT_DISCONNECT);
	PyModule_AddIntConstant(module, "MAGIC_RECV_DATA_FROM_SVR", MAGIC_RECV_DATA_FROM_SVR);
	PyModule_AddIntConstant(module, "MAGIC_RECV_DATA_FROM_CLIENT", MAGIC_RECV_DATA_FROM_CLIENT);
}

PyMODINIT_FUNC
initmagicnet(void)
{
	PyObject* pyModule ;
	pyModule = Py_InitModule("magicnet", Methods) ;
	AddIntConstant(pyModule) ;
}






