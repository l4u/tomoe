#include <pygobject.h>
#include "tomoe.h"

void pytomoe_register_classes (PyObject *d);

extern PyMethodDef pytomoe_functions[];

DL_EXPORT(void)
inittomoe(void)
{
    PyObject *m, *d;

    init_pygobject ();

    m = Py_InitModule ("tomoe", pytomoe_functions);
    d = PyModule_GetDict (m);

    pytomoe_register_classes (d);

    if (PyErr_Occurred ())
    {
        Py_FatalError ("can't initialize module globalkeys");
    }

    tomoe_init ();
    atexit (tomoe_quit);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
