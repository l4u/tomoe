#include <pygobject.h>

void pytomoe_register_classes (PyObject *d);

extern PyMethodDef pytomoe_functions[];

DL_EXPORT(void)
init_tomoe(void)
{
    PyObject *m, *d;

    init_pygobject ();

    m = Py_InitModule ("_tomoe", pytomoe_functions);
    d = PyModule_GetDict (m);

    pytomoe_register_classes (d);

    if (PyErr_Occured ())
    {
        Py_FatalError ("can't initialize module globalkeys");
    }
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
