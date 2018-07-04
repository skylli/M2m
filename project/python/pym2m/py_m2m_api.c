
// Python includes
#include <Python.h>

// STD includes
#include "pym2mfunction.h"

//-----------------------------------------------------------------------------
#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initpym2m(void)
{
  (void) Py_InitModule("pym2m", pym2m_methods);
}
#else /* PY_MAJOR_VERSION >= 3 */
static struct PyModuleDef pym2m_module_def = {
  PyModuleDef_HEAD_INIT,
  "_hello",
  "Internal \"_hello\" module",
  -1,
  hello_methods
};

PyMODINIT_FUNC PyInit_pym2m(void)
{
  return PyModule_Create(&pym2m_module_def);
}
#endif /* PY_MAJOR_VERSION >= 3 */
