
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>

#include <Python/Python.h>

#define PYTHONLIB_PATH "/System/Library/Frameworks/Python.framework/Python"

#define SIZEOF_STATIC_ARRAY( a ) ( sizeof((a)) / sizeof((a)[0]) )

typedef struct
{
	void (* Py_Initialize)(void);
	void (* Py_Finalize)(void);
	void (* PySys_SetArgv)(int, char **);
	void (* PySys_SetPath)(char *);
	int  (* PyRun_SimpleStringFlags)(const char *, PyCompilerFlags *);
}
python_functions_t;

char *pythonArgv[] = {
	"arg0",
	"argument 1",
	"something else",
	"forth",
};

const char *pythonScript =
"import sys\n"
"print sys.argv\n"
;

int main()
{
	// **** THE EASY WAY :-) ****
	
	//	Py_Initialize();
	//	PyRun_SimpleString("print \"Hello, world!\"");
	//	Py_Finalize();
	
	// **** THE HARD WAY :-\ ****
	
	//
	// WARNING: Should use NSCreateObjectFileImageFromFile instead
	// http://developer.apple.com/mac/library/technotes/tn2002/tn2071.html#Section6
	//
	void *pythonLib = dlopen(PYTHONLIB_PATH, RTLD_NOW);
	if (!pythonLib) {
		char *error = dlerror();
		fprintf(stderr, "dlopen() failed: %s\n", error);
		return 1;
	}
	
	python_functions_t python = {0};
#define LOAD_SYMBOL( symname ) if (!(python.symname = dlsym( pythonLib, #symname ))) { return -1; }
	LOAD_SYMBOL( Py_Initialize );
	LOAD_SYMBOL( Py_Finalize );
	LOAD_SYMBOL( PySys_SetArgv );
	LOAD_SYMBOL( PySys_SetPath );
	LOAD_SYMBOL( PyRun_SimpleStringFlags );
	
	python.Py_Initialize();
	python.PySys_SetArgv(SIZEOF_STATIC_ARRAY(pythonArgv), pythonArgv);
	python.PyRun_SimpleStringFlags(pythonScript, NULL);
	python.Py_Finalize();
	
	dlclose(pythonLib);
	
	return 0;
}
