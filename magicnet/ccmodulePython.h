#ifndef __SE_MAGICNET_PYTHON_H__
#define __SE_MAGICNET_PYTHON_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include <Python.h>
#include "SeMagicNet.h"
#include "SeTime.h"

static struct SEMAGICNETS kMagicNetGate;
static struct SEMAGICNETC kMagicNetSvr;

PyObject *MagicNetGateInit(PyObject *module, PyObject* args);

PyObject *MagicNetGateFin(PyObject *module, PyObject* args);

PyObject *MagicNetGateProcess(PyObject *module, PyObject* args);

PyObject *MagicNetSvrInit(PyObject *module, PyObject* args);

PyObject *MagicNetSvrFin(PyObject *module, PyObject* args);

PyObject *MagicNetSvrReg(PyObject *module, PyObject* args);

PyObject *MagicNetSvrSendClient(PyObject *module, PyObject* args);

PyObject *MagicNetSvrBindClient(PyObject *module, PyObject* args);

PyObject *MagicNetSvrCloseClient(PyObject *module, PyObject* args);

PyObject *MagicNetSvrSendSvr(PyObject *module, PyObject* args);

PyObject *MagicNetSvrRead(PyObject *module, PyObject* args);

PyObject *MagicTimeSleep(PyObject *module, PyObject* args);

PyObject *MagicTimeGetTickCount(PyObject *module, PyObject* args);

static void AddIntConstant(PyObject *module);

static PyMethodDef Methods[] = {
	//extern name, inner name, parameter passing , descrpition
	{"GateInit", MagicNetGateInit, METH_VARARGS, ""},
	{"GateFin", MagicNetGateFin, METH_VARARGS, ""},
	{"GateProcess", MagicNetGateProcess, METH_VARARGS, ""},
	{"SvrInit", MagicNetSvrInit, METH_VARARGS, ""},
	{"SvrFin", MagicNetSvrFin, METH_VARARGS, ""},
	{"RegSvr", MagicNetSvrReg, METH_VARARGS, ""},
	{"SvrSendClient", MagicNetSvrSendClient, METH_VARARGS, ""},
	{"SvrBindClient", MagicNetSvrBindClient, METH_VARARGS, ""},
	{"SvrCloseClient", MagicNetSvrCloseClient, METH_VARARGS, ""},
	{"SvrSendSvr", MagicNetSvrSendSvr, METH_VARARGS, ""},
	{"SvrRead", MagicNetSvrRead, METH_VARARGS, ""},
	{"TimeSleep", MagicTimeSleep, METH_VARARGS, ""},
	{"TickCount", MagicTimeGetTickCount, METH_VARARGS, ""},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};


PyMODINIT_FUNC initmagicnet(void);

#ifdef	__cplusplus
}
#endif

#endif
